#!/bin/bash

# Function to handle the termination of the loop on Ctrl + C
trap 'echo -e "\nProcess terminated."; break' SIGINT

# Inform the user that Ctrl + C will stop the loop
echo "Press Ctrl + C to stop the repeating beep process."

# Loop to print a beep after each iteration
while true; do
    # Run the weather program and pipe its output to tee to display in terminal and save to weather_data.txt
    ./weather_program | tee -a weather_data.txt

    # Send the bell character to terminal to generate a beep sound
    echo -e "\a"

    # Wait for 1 second between beeps (adjust as needed)
    sleep 5
done
