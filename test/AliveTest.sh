
echo "##4.send alive with correct api key and pincode"

mosquitto_pub -t "TM/ALIVE/dummy_device" -m "
{
   \"apikey\": \"1738f8e78548ef89d6bdd27ccda0fcfe61f393a50d6ea674\"
}"

sleep 1

mosquitto_pub -t "TM/ALIVE/dummy_device" -m "
{
   \"apikey\": \"1738f8e78548ef89d6bdd27ccda0fcfe61f393a50d6ea674\"
}"

sleep 1

