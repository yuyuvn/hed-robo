# hed-robo
A simple robo which do simple thing

### How to run
#### Install dependencies
* Install ROS: 
  * http://wiki.ros.org/indigo/Installation/Ubuntu
  * http://wiki.ros.org/ROS/Tutorials/InstallingandConfiguringROSEnvironment
* Install Turtlebot:
  * http://wiki.ros.org/turtlebot/Tutorials/indigo/Turtlebot%20Installation
  
#### Install simulator (if needed)
```
sudo apt-get install ros-indigo-turtlebot-apps ros-indigo-turtlebot-rviz-launchers
```
#### Install
```
cd ~/turtlebot/src
git clone https://github.com/yuyuvn/hed-robo.git
cd ..
source devel/setup.bash
catkin_make
```
#### Run
```
~/turtlebot/devel/lib/hedrobo/hedrobo
```
