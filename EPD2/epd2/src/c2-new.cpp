

#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <tf/tf.h>
#include <tf/transform_listener.h>

#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/PoseStamped.h>




#include <stdio.h>

#include <math.h>
#include <vector>
#include <fstream>


/**
* Our class to control the robot
* It has members to store the robot pose, and
* methods to control the robot by publishing data
*/
class Turtlebot
{
public:
  Turtlebot();
 
  /*
   * This function should command the robot to reach the goal
   * It should compute the commands to the robot by knowing the current position
   * and the goal position.
   * This function will return true only if the goal has been reached.
   */
  bool command(double goal_x, double goal_y);

private:

  
  ros::NodeHandle nh_;
  
  //2D robot pose
  double x,y,theta;

  //Publisher and subscribers
  ros::Publisher vel_pub_;
 

  //Transform listerner to obtain the transform between the world frame (odom) and the robot frame (base_link)
  tf::TransformListener listener;


  //!Publish the command to the turtlebot
  void publish(double angular_vel, double linear_vel);

};

Turtlebot::Turtlebot()
{
  vel_pub_ = nh_.advertise<geometry_msgs::Twist>("/cmd_vel_mux/input/navi", 1);

 

}




bool Turtlebot::command(double gx, double gy)  
{

	double linear_vel=0.0;
	double angular_vel=0.0;
	double dist;
  float ang = 0.0;
	bool ret_val = false;

	//Transform the goal to the local frame
	geometry_msgs::PointStamped goal;
	geometry_msgs::PointStamped base_goal;
	
  	goal.header.frame_id = "odom";

  	//we'll just use the most recent transform available for our simple example
  	goal.header.stamp = ros::Time();

  	//just an arbitrary point in space
  	goal.point.x = gx;
  	goal.point.y = gy;
  	goal.point.z = 0.0;

	try{
	    	listener.transformPoint("base_link", goal, base_goal);

	    	ROS_INFO("goal: (%.2f, %.2f. %.2f) -----> base_goal: (%.2f, %.2f, %.2f) at time %.2f",
		goal.point.x, goal.point.y, goal.point.z,
		base_goal.point.x, base_goal.point.y, base_goal.point.z, base_goal.header.stamp.toSec());

  	}catch(tf::TransformException& ex){
    		ROS_ERROR("Received an exception trying to transform a point from \"base_laser\" to \"base_link\": %s", ex.what());
		return false; 
  	}
	
	/**
	* This should be completed. You should use a proportional controller
	* The linear velocity should be proportional to the distance to the goal
	* The angular velocity should be proportional to the difference in the angle towards
	* the goal and the current angle of the robot. You should check if you reached the goal
	*/
	//calculate angle if is minimal then we are looking towards the goal
  
  ang = atan2(base_goal.point.y, base_goal.point.x);
  ang = ang*180/M_PI;
  
  if(ang > 5.0){
    angular_vel = 0.5;
	    	//ROS_INFO("ang:");
    linear_vel = 0.0;
  }else if (ang < -5.0){
    angular_vel = -0.5;
    linear_vel = 0.0;
  }
  //calculate the module of base_goal to know distast towards objective
   dist = sqrt(pow(base_goal.point.x, 2) + pow(base_goal.point.y, 2));
  
	if(abs(dist < 0.5)){
		linear_vel =0.0;
    angular_vel=0.0;
    ret_val = true;
	}else{
    linear_vel = 0.5;
  }


        publish(angular_vel,linear_vel);    


  	return ret_val;
}



//Publish the command to the turtlebot
void Turtlebot::publish(double angular, double linear)  
{
    geometry_msgs::Twist vel;
    vel.angular.z = angular;
    vel.linear.x = linear;

	//std::cout << "Velocidades: " << vel.linear.x << ", " << vel.angular.z << std::endl;

    vel_pub_.publish(vel);    


  return;
}




std::vector<geometry_msgs::Pose> loadPlan(const char *filename);

int main(int argc, char** argv)
{
  ros::init(argc, argv, "robot_control");
  Turtlebot robot;
  ros::NodeHandle n;
  int goalx, goaly;

  if(argc<2)
  {
	std::cout << "Insufficient number of parameters" << std::endl;
	std::cout << "Usage: robot_control <filename>" << std::endl;
	return 0;
  }

  std::vector<geometry_msgs::Pose> plan = loadPlan(argv[1]);
  unsigned int cont_wp = 0;
  bool aux = false;
  ros::Rate loop_rate(20);

  /**
  * Complete the code to follow the path,
  * calling adequately to the command function
  */

  //call command here
  while (ros::ok() && cont_wp<3)
  {
   
    goalx = plan[cont_wp].position.x;
    goaly = plan[cont_wp].position.y;
    
    aux = robot.command(goalx, goaly);
    if(aux){
      cont_wp++;
      aux = false;
    }
    loop_rate.sleep();
  }

  return 0;

}

std::vector<geometry_msgs::Pose> loadPlan(const char *filename) {
  std::vector<geometry_msgs::Pose> plan;
  double x,y;
  
  std::ifstream is(filename);
  
  while (is.good()) {
    is >> x;
    if (is.good()) {
      is >> y;
      geometry_msgs::Pose curr_way;
      curr_way.position.x = x;
      curr_way.position.y = y;
      plan.push_back(curr_way);
      ROS_INFO("Loaded waypoint (%f, %f).", x , y);
    }
  }
  ROS_INFO("Plan loaded successfully.");
  return plan;
}


