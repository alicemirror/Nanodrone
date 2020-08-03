/** 

The tenasorflow part of this source is original copyright 2019 
of The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Starting from the Arducam Pro 2.0 Mp the detection of a person is sent via BLE
to the ground control.

This application is the proof of concept of the Nanodrone application aimed to
retrieve sensor information from the flying drone and collect them on a remotely
connected applicaiton based on Ble for short range communication.

*/

#include <TensorFlowLite.h>
#include <ArduinoBLE.h>

#include "main_functions.h"

#include "detection_responder.h"
#include "image_provider.h"
#include "model_settings.h"
#include "person_detect_model_data.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#undef _REPORT_TF_LITE_ERROR

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;

// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 93 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

//! Create the custom BLE service with a random generated UUID
BLEService personDetect("4f375fe5-24b2-46ba-b760-5b121ec695df");
//! Define the custom characteristic with the associated UUID
//! Clients are notified when the characteristic changes and can read the value
BLEUnsignedCharCharacteristic humanDetected("4f375fe5-24b2-46ba-b760-5b121ec695df",
    BLERead | BLENotify);
//! Clients receives the number of tries
BLEUnsignedCharCharacteristic triesCount("4f375fe5-24b2-46ba-b760-5b121ec695df",
    BLERead | BLENotify);

//! Number of tries to detect a human
int detections = 0;

// The name of this function is important for Arduino compatibility.
void setup() {
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_person_detect_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
#ifdef _REPORT_TF_LITE_ERROR
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
#endif
    return;
  }

  // Start BLE 
  while(!BLE.begin()) {
    delay(100);
  }

  // Initialize the bLE features before starting advertising
  BLE.setLocalName("Nanodrone");
  BLE.setAdvertisedService(personDetect);
  personDetect.addCharacteristic(humanDetected);
  personDetect.addCharacteristic(triesCount);
  BLE.addService(personDetect);
  BLE.addService(personDetect);
  // The initial value is false, as no person has been detected
  humanDetected.writeValue(false);
  triesCount.writeValue((int)0);

  /* Start advertising BLE.  It will start continuously transmitting BLE
     advertising packets and will be visible to remote BLE central devices
     until it receives a new connection */

  // start advertising
  BLE.advertise();

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  //
  // tflite::ops::micro::AllOpsResolver resolver;
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroOpResolver<3> micro_op_resolver;
  micro_op_resolver.AddBuiltin(
      tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
      tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                               tflite::ops::micro::Register_CONV_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_AVERAGE_POOL_2D,
                               tflite::ops::micro::Register_AVERAGE_POOL_2D());

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
#ifdef _REPORT_TF_LITE_ERROR
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
#endif
    return;
  }

  // Get information about the memory area to use for the model's input.
  input = interpreter->input(0);

}

// The name of this function is important for Arduino compatibility.
void loop() {
  BLEDevice central = BLE.central();

    // Get image from provider.
    if (kTfLiteOk != GetImage(error_reporter, kNumCols, kNumRows, kNumChannels,
                              input->data.uint8)) {
#ifdef _REPORT_TF_LITE_ERROR
      TF_LITE_REPORT_ERROR(error_reporter, "Image capture failed.");
#endif
    }
  
    // Run the model on this input and make sure it succeeds.
    if (kTfLiteOk != interpreter->Invoke()) {
#ifdef _REPORT_TF_LITE_ERROR
      TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
#endif
    }
  
    TfLiteTensor* output = interpreter->output(0);
  
    // Process the inference results.
    uint8_t person_score = output->data.uint8[kPersonIndex];
    uint8_t no_person_score = output->data.uint8[kNotAPersonIndex];
    RespondToDetection(error_reporter, person_score, no_person_score);
    // Advertise the detection, if any
    // Here we consider only the highest score but it is worth to optimize the
    // detection adding the trustability of the result
    // Note that only the detected events are notified.
    if(person_score > 150) {
      humanDetected.writeValue('T');
    } // Human has been detected!
    else {
      humanDetected.writeValue('F');
    } // No human detected

    triesCount.writeValue((unsigned char)++detections);

}
