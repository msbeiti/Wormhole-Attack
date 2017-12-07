iw dev adhoc del
iw dev wlan0 del
iw dev mesh del
rfkill unblock all
iw phy phy0 interface add wlan0 type ibss
iw reg set DE
ifconfig wlan0 down
echo "2"
iwconfig wlan0 mode monitor
echo "3"
iw dev wlan0 set channel 6  #HT40+
echo "3.5"
iw dev wlan0 set txpower fixed 1400
echo "4"
ifconfig wlan0 mtu 1600
echo "5"
ifconfig wlan0 up
