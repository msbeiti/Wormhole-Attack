iw dev adhoc del
iw dev wlan1 del
iw dev mesh del
export ADHOC_IFACE=adhoc
export ADHOC_ID=AVIGLE
rfkill unblock all
echo "1"
#iw phy phy2 interface add $ADHOC_IFACE type managed
iw phy phy1 interface add $ADHOC_IFACE type ibss
echo "2"
ifconfig adhoc hw ether 00:0b:6b:02:02:86
#echo "3"
#iw dev adhoc set type adhoc
echo "4"
ifconfig $ADHOC_IFACE up
echo "4.5"
iw dev $ADHOC_IFACE ibss join $ADHOC_ID 2437 #HT40+
iw dev $ADHOC_IFACE set channel 6 #HT40+
echo "5"
#ifconfig $ADHOC_IFACE up
echo "6"
#iw dev $ADHOC_IFACE station dump
iw dev $ADHOC_IFACE set txpower fixed 1800
echo "7"
#ifconfig $ADHOC_IFACE 192.168.33.102
#route add default gw 192.168.33.101 adhoc
#iw dev $ADHOC_IFACE scan

#echo "1" > /proc/sys/net/ipv4/ip_forward
