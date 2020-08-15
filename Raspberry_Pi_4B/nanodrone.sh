#!/bin/bash
# Nanodrone v. 1.0 PiJuice automatic start
#
# This command starts automatically the nanodrone application. The command
# is associated to the custom switch 1 of the PiJuich UPS battery to start
# the application on demand (do it before flying)

echo "Launching the Nanodrone application"
cd ~/nanodrone
./firstfly &
echo "Application started."
