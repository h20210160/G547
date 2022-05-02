
# Connecting RaspberryPi in Master Mode to Arduino in Slave mode using I2C protocol and GPIO subsystem

I²C (Inter-Integrated Circuit), pronounced I-squared-C, is a synchronous, multi-master, multi-slave, packet switched, single-ended, serial computer bus invented in 1982 by Philips Semiconductor (now NXP Semiconductors). It is widely used for attaching lower-speed peripheral ICs to processors and microcontrollers in short-distance, intra-board communication. Alternatively, I²C is spelled I2C (pronounced I-two-C) or IIC (pronounced I-I-C) as per the Wikipedia page.
The i2C uses two lines to send and receive data. We use Serial Clock Pin (SCL) for the synchronous clock and Serial Data Pin (SDA) for the data transfer.

We can have 128 Multiple master and slave connected in a single i2C bus if it is a 7-bit address bus.
The data transferred in binary message format and that format are broken up into frames of data. Each message has a start condition, address frame or control frame, data frame, stop condition.

### Start condition:
The master leaves SCL High and pulls SDA low and puts all the slave devices on notice that a data transfer is about to start.
### Control frame:
The control frame also knows as a slave device address followed by Read/Write bit indicating this operation Read(1) or Write(0).
We send a slave address that the master wants to communicate over the SDA bus and each slave device compares this address with their address sends the ACK or NACK if it matches. Now all set to communicate between the master and the slave device.
### Data Frame:
Now master sends an 8 bits data to slave or slave sends data to master depending on the control bit Read or Write.
### Stop Condition:
Once all the data frames send, the master will generate a Stop condition and saying that I’m done with a transfer.

In our project we are going to set up the i2c communication between Raspberry PI 3 (Master )and an Arduino (Slave).

# Hardware Schematics:

![alt text](https://miro.medium.com/max/1400/1*8pAaHVNdR9k7IBE5CyEFIg.png)
