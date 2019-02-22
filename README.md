# fall-detection-iot
this project detect the fall of its holder and send the status to thinkspeak web cloud then we can generate file describe a certain period.

the algorithm used in this project is threshold that checks in More than a stage to avoid errors produced by the fast motion of device holder. 

i'm using nodeMCU in this project and mpu6050 as a sensor of motion.

And i'm using wire.h library to transact with the I2C serial communication protocol , and esp library to transact with the web client.
