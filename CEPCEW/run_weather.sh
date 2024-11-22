#!/bin/bash

#  termination of the loop on Ctrl + C
trap 'echo -e "\nProcess terminated."; break' SIGINT


echo "Press Ctrl + C to stop the repeating beep process."


while true; do

    ./weather_program | tee -a weather_data.txt

   #bell sound
    echo -e "\a"

  #Time delay
    sleep 5
done
