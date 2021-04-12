EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L RF_Module:ESP-07 U?
U 1 1 606231B3
P 3550 2950
F 0 "U?" H 3550 3931 50  0000 C CNN
F 1 "ESP-07" H 3550 3840 50  0000 C CNN
F 2 "RF_Module:ESP-07" H 3550 2950 50  0001 C CNN
F 3 "http://wiki.ai-thinker.com/_media/esp8266/esp8266_series_modules_user_manual_v1.1.pdf" H 3200 3050 50  0001 C CNN
	1    3550 2950
	1    0    0    -1  
$EndComp
$Comp
L Device+:PZEM-004T U?
U 1 1 60625E2C
P 2950 1350
F 0 "U?" H 3525 1965 50  0000 C CNN
F 1 "PZEM-004T" H 3525 1874 50  0000 C CNN
F 2 "" H 2950 1350 50  0001 C CNN
F 3 "" H 2950 1350 50  0001 C CNN
	1    2950 1350
	1    0    0    -1  
$EndComp
Wire Wire Line
	4100 1150 4350 1150
Wire Wire Line
	4350 1150 4350 3150
Wire Wire Line
	4350 3150 4150 3150
Wire Wire Line
	4100 1250 4450 1250
Wire Wire Line
	4450 1250 4450 2950
Wire Wire Line
	4450 2950 4150 2950
Wire Wire Line
	3550 3650 3550 3800
Wire Wire Line
	3550 3800 4650 3800
Wire Wire Line
	4650 3800 4650 1350
Wire Wire Line
	4650 1350 4100 1350
Wire Wire Line
	2950 1050 2300 1050
Wire Wire Line
	2950 1150 1400 1150
Wire Wire Line
	2950 1250 2300 1250
Wire Wire Line
	2300 1250 2300 1050
Connection ~ 2300 1050
Wire Wire Line
	2300 1050 1400 1050
Wire Wire Line
	2950 1350 1400 1350
$Comp
L Display_Character:NHD-0420H1Z U?
U 1 1 6062C5CA
P 6800 2750
F 0 "U?" H 6800 1861 50  0000 C CNN
F 1 "NHD-0420H1Z" H 6800 1770 50  0000 C CNN
F 2 "Display:NHD-0420H1Z" H 6800 1850 50  0001 C CNN
F 3 "http://www.newhavendisplay.com/specs/NHD-0420H1Z-FSW-GBW-33V3.pdf" H 6900 2650 50  0001 C CNN
	1    6800 2750
	1    0    0    -1  
$EndComp
$EndSCHEMATC
