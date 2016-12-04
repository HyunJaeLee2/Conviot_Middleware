echo "##1.Register non existing thing"
mosquitto_pub -t "TM/REGISTER/mydummydevice" -m "
{
   \"pincode\": \"customer1_pin_code_1\",
   \"apikey\": \"1738f8e78548ef89d6bdd27ccda0fcfe61f393a50d6ea674+12341324\",
}"

sleep 1

echo "##2.Register thing with incorrect api key"
mosquitto_pub -t "TM/REGISTER/customer1_device_id_1" -m "
{
   \"pincode\": \"customer1_pin_code_1\",
   \"apikey\": \"1738f8e78548ef89d6bdd27ccda0fcfe61f393a50d6ea674+12341324\",
}"

sleep 1

echo "##3.Register thing with non existing pincode"
mosquitto_pub -t "TM/REGISTER/customer1_device_id_1" -m "
{
   \"pincode\": \"customer1_pin_code_1+1234\",
   \"apikey\": \"1738f8e78548ef89d6bdd27ccda0fcfe61f393a50d6ea674\",
}"

sleep 1

echo "##4.Register thing with correct api key and pincode"

mosquitto_pub -t "TM/REGISTER/customer1_device_id_1" -m "
{
   \"pincode\": \"customer1_pin_code_1\",
   \"apikey\": \"1738f8e78548ef89d6bdd27ccda0fcfe61f393a50d6ea674\",
}"

sleep 1
#
#mosquitto_pub -t "TM/REGISTER/thing3" -m "
#{
#   \"api_key\": \"aslkgjlaksdjgl\",
#}"
#
#sleep 1
#
#echo "##2. unregister thing "
#sleep 2
#mosquitto_pub -t "TM/UNREGISTER/thing3" -m "
#{
#   \"api_key\": \"aslkgjlaksdjgl\",
#}"
#
#sleep 1
#mosquitto_pub -t "TM/UNREGISTER/thing1" -m "
#{
#   \"api_key\": \"aslkgjlaksdjgl\",
#}"
#sleep 1
#
#echo "##3. alive thing"
#sleep 2
#mosquitto_pub -t "TM/ALIVE/thing3" -m "
#{
#   \"api_key\": \"aslkgjlaksdjgl\",
#}"
#
#sleep 1
#mosquitto_pub -t "TM/ALIVE/thing1" -m "
#{
#   \"api_key\": \"aslkgjlaksdjgl\",
#}"
#sleep 1
#
#echo "##4. send variable"
#sleep 2
#mosquitto_pub -t "TM/SEND_VARIABLE/thing3/motion" -m "
#{
#   \"value\": \"dummy\",
#   \"api_key\": \"aslkgjlaksdjgl\",
#}"
#
#sleep 1
#mosquitto_pub -t "TM/SEND_VARIABLE/thing1/temperature" -m "
#{
#   \"value\": \"13\",
#   \"api_key\": \"aslkgjlaksdjgl\",
#}"
#sleep 1

#echo "##2. register duplicated thing(error message will show up) "
#sleep 3
#mosquitto_pub -t "TM/REGISTER/thing3" -m "
#{
#   \"alive_cycle\": \"30\",
#   \"description\": \"temp\",
#   \"values\":[
#                 {
#                   \"name\":\"isLabMember\",
#                   \"type\":\"int\",
#                   \"bound\":{
#                               \"min_value\":\"-1\",
#                               \"max_value\":\"1\"
#                  },
#                 }
#             ],
#   \"functions\":[]
#}"
#
#sleep 1
#
#
#mosquitto_pub -t "TM/REGISTER/thing4" -m "
#{
#   \"alive_cycle\": \"30\",
#   \"description\": \"temp\",
#   \"values\":[],
#   \"functions\":[
#                      {
#                        \"name\":\"SecurityAlert\",
#                        \"useArg\":\"1\",
#                        \"functionArgType\":\"bool\",
#                        \"functionArgBound\":{\"min_value\":\"0\",
#                                              \"max_value\":\"1\"},
#                        }
#                   ]
#}"
#
#echo "##3. unregister then re-register thing "
#sleep 3
#mosquitto_pub -t "TM/UNREGISTER/thing3" -m ""
#sleep 1
#mosquitto_pub -t "TM/UNREGISTER/thing4" -m ""
#sleep 1
#
#mosquitto_pub -t "TM/REGISTER/thing3" -m "
#{
#   \"alive_cycle\": \"30\",
#   \"description\": \"temp\",
#   \"values\":[
#                 {
#                   \"name\":\"isLabMember\",
#                   \"type\":\"int\",
#                   \"bound\":{
#                               \"min_value\":\"-1\",
#                               \"max_value\":\"1\"
#                  },
#                 }
#             ],
#   \"functions\":[]
#}"
#
#sleep 1
#
#
#mosquitto_pub -t "TM/REGISTER/thing4" -m "
#{
#   \"alive_cycle\": \"30\",
#   \"description\": \"temp\",
#   \"values\":[],
#   \"functions\":[
#                      {
#                       \"name\":\"PrintString\",
#                       \"useArg\":\"1\",
#                       \"functionArgType\":\"string\",
#                       \"functionArgBound\":{\"min_value\":\"0\",
#                                             \"max_value\":\"10\"},
#                      },
#                      {
#                        \"name\":\"SecurityAlert\",
#                        \"useArg\":\"1\",
#                        \"functionArgType\":\"bool\",
#                        \"functionArgBound\":{\"min_value\":\"0\",
#                                              \"max_value\":\"1\"},
#                        }
#                   ]
#}"
#
#
#echo "##4. Add scenarios"
#sleep 3
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "SeatingHelloAlramOn#[0s](thing1.onSeatTime > 30){thing2.PrintHello()}"
#sleep 1
#
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "SeatingAlramOn#[0s](thing1.onSeatTime >= 30){thing2.Alarm(1)}"
#sleep 1
#
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "SeatingAlramOff#[10s](thing1.onSeatTime < 30){thing2.Alarm(0)}"
#sleep 1
#
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "SecurityAlertOff#[1m](thing3.isLabMember == 0){thing4.SecurityAlert(0)}"
#sleep 1
#
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "SecurityAlertOn#[0s](thing3.isLabMember == -1){thing4.SecurityAlert(1)}"
#sleep 1
#
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "GreetingMessage#[0s](thing3.isLabMember == 1){thing4.PrintString(\"Welcome\")}"
#sleep 1
#
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "LightOff#[5s](thing5.joystick < -200){thing6.Light(0)}"
#sleep 1
#
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "LightOn#[5s](thing5.joystick > 300){thing6.Light(1)}"
#
#echo "##5. Register duplicated scenarios(error message will show up)"
#
#sleep 3
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "SecurityAlertOn#[0s](thing3.isLabMember == -1){thing4.SecurityAlert(1)}"
#sleep 1
#
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "GreetingMessage#[0s](thing3.isLabMember == 1){thing4.PrintString(\"Welcome\")}"
#
#echo "##6. Delete then re-add scenarios"
#
#sleep 3
#mosquitto_pub -t "EM/DELETE_SCENARIO/client1" -m "1"
#sleep 1
#mosquitto_pub -t "EM/DELETE_SCENARIO/client2" -m "4"
#sleep 1
#
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "SeatingHelloAlramOn#[0s](thing1.onSeatTime > 30){thing2.PrintHello()}"
#sleep 1
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "SecurityAlertOff#[10s](thing3.isLabMember == 0){thing4.SecurityAlert(0)}"
#
#echo "##7. add scenarios of wrong condition"
#
#sleep 3
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "WrongConditionScenario1#[0s](thing1.onSeatTime > 30000){thing2.PrintHello()}"
#sleep 1
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "WrongConditionScenario2#[0s](thing3.isLabMember == 200){thing2.PrintHello()}"
#
#echo "##8. add scenarios of wrong action"
#sleep 3
#
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "WrongActionScenario1#[5s](thing5.joystick > 30){thing6.Light(1000)}"
#sleep 1
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "WrongActionScenario2#[10s](thing5.joystick < -30){thing6.Light(abcd)}"
#
#echo "##9. Run Event-based Scenario with sensor value"
#
#sleep 3
#mosquitto_pub -t "thing1/onSeatTime" -m "{\"type\":\"double\", \"value\":\"40\"}"
#sleep 1
#
#mosquitto_pub -t "thing3/isLabMember" -m "{\"type\":\"int\", \"value\":\"-1\"}"
#sleep 1
#
#mosquitto_pub -t "thing3/isLabMember" -m "{\"type\":\"int\", \"value\":\"1\"}"
#
#echo "##10. Run time-based Scenario with sensor value"
#
#sleep 3
#mosquitto_pub -t "thing1/onSeatTime" -m "{\"type\":\"double\", \"value\":\"20\"}"
#sleep 1
#
#mosquitto_pub -t "thing5/joystick" -m "{\"type\":\"int\", \"value\":\"400\"}"
#sleep 1
#
#
#echo "##11. Actuate Test"
#sleep 3
#mosquitto_pub -t "EM/ACTUATE/client1" -m "thing4.SecurityAlert(1)"
#sleep 1
#mosquitto_pub -t "EM/ACTUATE/client1" -m "thing4.PrintString(Welcome)"
#
#echo "##12. Refresh Test"
#sleep 3
#mosquitto_pub -t "EM/REFRESH/client1" -m " "
#
#echo "##13.Multiple Condition Test"
#sleep 3
#mosquitto_pub -t "EM/ADD_SCENARIO/client1" -m "MultipleConditionTest#[0s](thing1.onSeatTime > 30 && (thing3.isLabMember == 0 || thing5.joystick > 30 && thing5.joystick < 100) || thing3.isLabMember == -1 ){thing2.PrintHello() ; thing2.Alarm(1)}"
#
#echo "##14.Binary Message Test"
#
#sleep 3
#mosquitto_pub -t "thing1/isSeated" -m "{\"type\":\"binary\",\"format\":\"jpeg\", \"value\":\"VGhpcyBpcyBiYXNlNjQgdGVzdA==\"}"
#
#echo "##14.Set Virtual Thing Id Test"
#sleep 3
#
#mosquitto_pub -t "EM/SET_THING_ID/client1" -m "{\"old_id\":\"thing2\", \"new_id\":\"mytest\"}"
