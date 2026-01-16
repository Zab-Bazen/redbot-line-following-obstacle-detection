# RedBot README
# RedBot Line Following + Obstacle Detection (C/C++)

This project controls a SparkFun RedBot to follow a line using 3 reflectance sensors (left/center/right) and detect obstacles using a SharpIR distance sensor.

The robot:
- Prints live sensor readings to the Serial Monitor
- Follows a line using threshold-based logic
- Stops and performs a maneuver when an object is detected within a distance range

## Tech Used
- Arduino-style C/C++
- SparkFun RedBot Library (`RedBotSensor`, `RedBotMotors`)
- SharpIR Library (`SharpIR.distance()`)

## Project Behavior
### Line Following
- If the center sensor detects the line → drive forward
- If the left/right sensor detects the line → adjust motor speeds to steer back onto the line

### Obstacle Detection
- Uses SharpIR distance readings to detect nearby objects
- Stops and runs a short avoidance sequence when an object is detected in range

## Serial Telemetry
The program prints:
- IR distance measurement
- Time taken to measure distance
- Raw left/center/right sensor readings

This helps with calibration and debugging.

## How to Run
1. Install Arduino IDE
2. Install the RedBot + SharpIR libraries
3. Upload code to RedBot
4. Open Serial Monitor at **9600 baud**

## Demo Video
https://www.youtube.com/watch?v=KNL2H40OZEw
