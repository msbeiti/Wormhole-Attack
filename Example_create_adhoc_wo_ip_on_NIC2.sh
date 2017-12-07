iw dev adhoc del
iw dev wlan1 del
iw dev mesh del
export ADHOC_IFACE=adhoc
export ADHOC_ID=VICTIM
rfkill unblock all
echo "1"
iw phy phy1 interface add $ADHOC_IFACE type ibss
echo "2"
ifconfig adhoc hw ether 00:0b:6b:02:02:86
echo "3"
ifconfig $ADHOC_IFACE up
echo "3.5"
iw dev $ADHOC_IFACE ibss join $ADHOC_ID 2437 #HT40+
echo "4"
#iw dev $ADHOC_IFACE station dump
iw dev $ADHOC_IFACE set txpower fixed 1800
echo "5"
#iw dev $ADHOC_IFACE scan

