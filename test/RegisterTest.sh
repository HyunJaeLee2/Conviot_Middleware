echo "##1.Register non existing thing"
mosquitto_pub -t "TM/REGISTER/mydummydevice" -m "
{
   \"pincode\": \"customer1_pin_code_1\",
   \"apikey\": \"1738f8e78548ef89d6bdd27ccda0fcfe61f393a50d6ea674+12341324\",
}"

sleep 2

echo "##2.Register thing with incorrect api key"
mosquitto_pub -t "TM/REGISTER/customer1_device_id_1" -m "
{
   \"pincode\": \"customer1_pin_code_1\",
   \"apikey\": \"1738f8e78548ef89d6bdd27ccda0fcfe61f393a50d6ea674+12341324\",
}"

sleep 2

echo "##3.Register thing with non existing pincode"
mosquitto_pub -t "TM/REGISTER/customer1_device_id_1" -m "
{
   \"pincode\": \"customer1_pin_code_1+1234\",
   \"apikey\": \"1738f8e78548ef89d6bdd27ccda0fcfe61f393a50d6ea674\",
}"

sleep 2

echo "##4.Register thing with correct api key and pincode"

mosquitto_pub -t "TM/REGISTER/customer1_device_id_1" -m "
{
   \"pincode\": \"customer1_pin_code_1\",
   \"apikey\": \"1738f8e78548ef89d6bdd27ccda0fcfe61f393a50d6ea674\",
}"

sleep 2


echo "##5.Duplicated Register Request"

mosquitto_pub -t "TM/REGISTER/customer1_device_id_1" -m "
{
   \"pincode\": \"customer1_pin_code_1\",
   \"apikey\": \"1738f8e78548ef89d6bdd27ccda0fcfe61f393a50d6ea674\",
}"

sleep 2
