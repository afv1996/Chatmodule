#!/bin/bash
PATHLINK="/home/nhatanh/Desktop/chatting_in_kernel/"
CURRENTPID="$1"
echo "NOTICE!!!"
echo "If you are a new comer, Please choose 1 to join chat_module"
echo "If your username have already exists, Please choose 2 to change address PID"
echo "Choose option"
echo "1-Join"
echo "2-Rejoin"
echo "3-Show all user"
echo "4-Chat with user"
echo "5-Change username"
echo "6-Change status"
echo "7-Noti all user"
echo "8-Exit"
while :
do
	read INPUT
	case $INPUT in
		1)
			echo "***********************************"
			read -ep "Enter your username >" USERNAME
			echo "$USERNAME/$CURRENTPID" > /proc/Chat_Add_Pid
			echo "***********************************"
			;;
		2)
			echo "***********************************"
			read  -ep "Enter your username > " OLDUSERNAME
			./ioctl_test 1 "$OLDUSERNAME/$CURRENTPID"
			echo "***********************************"
			;;
		3)
			echo "***********************************"
			cat /proc/Chat_Add_Pid
			echo "***********************************"
			;;	
		4)
			echo "***********************************"
			cat /proc/Chat_Add_Pid
			read -ep "Enter username of the other > " DESNAME
			read -ep "Enter the PID of user > " DESPID
			LINK="$PATHLINK$CURRENTPID-$DESNAME.sh"
			echo '#!/bin/bash' >> $LINK
			echo "IFS=''" >> $LINK
			echo 'QUIT="GBTW"' >> $LINK
			echo 'MESS="$1"' >> $LINK
			echo "echo "$DESNAME/$DESPID/\$MESS" > /proc/Chat_Mess" >> $LINK
		        echo "Enter $CURRENTPID-$DESNAME.sh \"[YOURMESS]\" to chat with the other"
			chmod 777 $LINK
			#echo 'while : ' >> $LINK
			#echo 'do' >> $LINK
			#echo 'read -ep "YOU > " MESS' >> $LINK
			#echo 'if [ $MESS == $QUIT ]; then' >> $LINK
			#echo 'break' >> $LINK
			#echo 'else' >> $LINK
			#echo "echo "$DESNAME/$DESPID/\$MESS" > /proc/Chat_Mess" >> $LINK
			#echo "fi" >> $LINK
			#echo "done" >> $LINK
			echo "***********************************"
			;;
			

		5)
			echo "***********************************"
			read -ep "Enter [oldusername/newusername] > " NEWUSERNAME
			./ioctl_test 2 "$NEWUSERNAME"
			echo "***********************************"
			;;	
		6)
			echo "***********************************"
			read -ep "Enter Username/Status > " CHANGESTATUS
			./ioctl_test 2 "$CHANGESTATUS"
			echo "***********************************"
			;;
		7)
			echo "***********************************"
			read -ep "Enter Mess > " MESS
			echo "all/$MESS" > /proc/Chat_Mess
			echo "***********************************"
			;;
		8)
			break
	esac
done
echo
echo "Bye"
