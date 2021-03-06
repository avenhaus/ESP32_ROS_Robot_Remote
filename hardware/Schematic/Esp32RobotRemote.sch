EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "ESP32 ROS Robot Remote"
Date "2021-12-19"
Rev "0.5"
Comp "Carsten Avenhaus"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Espressif:ESP32-DevKitC U1
U 1 1 61B6C949
P 2900 2050
F 0 "U1" H 2975 3215 50  0000 C CNN
F 1 "ESP32-DevKitC" H 2975 3124 50  0000 C CNN
F 2 "Espressif:ESP32-DevKitC" H 2900 800 50  0001 C CNN
F 3 "https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/hw-reference/esp32/get-started-devkitc.html" H 3050 800 50  0001 C CNN
	1    2900 2050
	1    0    0    -1  
$EndComp
$Comp
L CarstenKiCadLibrary:TFT_SPI_ILI9xxx U2
U 1 1 61B7231E
P 9700 4100
F 0 "U2" H 9350 4650 50  0000 C CNN
F 1 "TFT_SPI_ILI9xxx" H 10300 4650 50  0000 C CNN
F 2 "" H 9750 4100 50  0001 C CNN
F 3 "" H 9750 4100 50  0001 C CNN
	1    9700 4100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61B74380
P 9950 5200
F 0 "#PWR?" H 9950 4950 50  0001 C CNN
F 1 "GND" H 9955 5027 50  0000 C CNN
F 2 "" H 9950 5200 50  0001 C CNN
F 3 "" H 9950 5200 50  0001 C CNN
	1    9950 5200
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 61B75828
P 1400 950
F 0 "#PWR?" H 1400 800 50  0001 C CNN
F 1 "+3.3V" H 1415 1123 50  0000 C CNN
F 2 "" H 1400 950 50  0001 C CNN
F 3 "" H 1400 950 50  0001 C CNN
	1    1400 950 
	1    0    0    -1  
$EndComp
Wire Wire Line
	1600 1200 1400 1200
Wire Wire Line
	1400 1200 1400 950 
Text GLabel 9200 3850 0    50   Input ~ 0
RST
Text GLabel 1600 1300 0    50   Input ~ 0
RST
Text GLabel 9200 4150 0    50   Input ~ 0
SCK
Text GLabel 9200 4050 0    50   Input ~ 0
MOSI
Text GLabel 9200 4550 0    50   Input ~ 0
SCK
Text GLabel 10600 4500 2    50   Input ~ 0
SCK
Text GLabel 4350 2000 2    50   Input ~ 0
SCK
Text GLabel 4350 1300 2    50   Input ~ 0
MOSI
Text GLabel 10600 4300 2    50   Input ~ 0
MOSI
Text GLabel 9200 4850 0    50   Input ~ 0
MISO
Text GLabel 10600 4400 2    50   Input ~ 0
MISO
Text GLabel 4350 1900 2    50   Input ~ 0
MISO
$Comp
L power:GND #PWR?
U 1 1 61B7798A
P 4800 1800
F 0 "#PWR?" H 4800 1550 50  0001 C CNN
F 1 "GND" H 4805 1627 50  0000 C CNN
F 2 "" H 4800 1800 50  0001 C CNN
F 3 "" H 4800 1800 50  0001 C CNN
	1    4800 1800
	1    0    0    -1  
$EndComp
Wire Wire Line
	4350 1800 4800 1800
Wire Wire Line
	4350 1200 4800 1200
Wire Wire Line
	4800 1200 4800 1800
Connection ~ 4800 1800
$Comp
L power:GND #PWR?
U 1 1 61B789FA
P 1050 2500
F 0 "#PWR?" H 1050 2250 50  0001 C CNN
F 1 "GND" H 1055 2327 50  0000 C CNN
F 2 "" H 1050 2500 50  0001 C CNN
F 3 "" H 1050 2500 50  0001 C CNN
	1    1050 2500
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 61B79365
P 1050 3000
F 0 "#PWR?" H 1050 2850 50  0001 C CNN
F 1 "+5V" H 1065 3173 50  0000 C CNN
F 2 "" H 1050 3000 50  0001 C CNN
F 3 "" H 1050 3000 50  0001 C CNN
	1    1050 3000
	1    0    0    -1  
$EndComp
Text GLabel 9200 3750 0    50   Input ~ 0
LCD_CS
Text GLabel 4350 2700 2    50   Input ~ 0
LCD_CS
Text GLabel 9200 3950 0    50   Input ~ 0
LCD_DC
Text GLabel 4350 2600 2    50   Input ~ 0
LCD_DC
Text GLabel 9200 4650 0    50   Input ~ 0
T_CS
Text GLabel 1600 2300 0    50   Input ~ 0
T_CS
Text GLabel 9200 4750 0    50   Input ~ 0
MOSI
Text GLabel 9200 4250 0    50   Input ~ 0
LCD_LED
Text GLabel 1600 2400 0    50   Input ~ 0
LCD_LED
Text GLabel 10600 4200 2    50   Input ~ 0
SD_CS
Text GLabel 4350 2100 2    50   Input ~ 0
SD_CS
$Comp
L 74xx:74HC165 U3
U 1 1 61B7C799
P 6400 2000
F 0 "U3" H 6150 2750 50  0000 C CNN
F 1 "74HC165" H 6650 2750 50  0000 C CNN
F 2 "Package_DIP:DIP-16_W7.62mm" H 6400 2000 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/74HC_HCT165.pdf" H 6400 2000 50  0001 C CNN
	1    6400 2000
	1    0    0    -1  
$EndComp
$Comp
L 74xx:74HC165 U4
U 1 1 61B7E61B
P 8100 2000
F 0 "U4" H 7850 2750 50  0000 C CNN
F 1 "74HC165" H 8350 2750 50  0000 C CNN
F 2 "Package_DIP:DIP-16_W7.62mm" H 8100 2000 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/74HC_HCT165.pdf" H 8100 2000 50  0001 C CNN
	1    8100 2000
	1    0    0    -1  
$EndComp
$Comp
L 74xx:74HC165 U5
U 1 1 61B7EE17
P 9950 2000
F 0 "U5" H 9700 2750 50  0000 C CNN
F 1 "74HC165" H 10200 2750 50  0000 C CNN
F 2 "Package_DIP:DIP-16_W7.62mm" H 9950 2000 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/74HC_HCT165.pdf" H 9950 2000 50  0001 C CNN
	1    9950 2000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61B812F5
P 6400 3000
F 0 "#PWR?" H 6400 2750 50  0001 C CNN
F 1 "GND" H 6405 2827 50  0000 C CNN
F 2 "" H 6400 3000 50  0001 C CNN
F 3 "" H 6400 3000 50  0001 C CNN
	1    6400 3000
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 61B822A9
P 6400 1100
F 0 "#PWR?" H 6400 950 50  0001 C CNN
F 1 "+3.3V" H 6415 1273 50  0000 C CNN
F 2 "" H 6400 1100 50  0001 C CNN
F 3 "" H 6400 1100 50  0001 C CNN
	1    6400 1100
	1    0    0    -1  
$EndComp
Wire Wire Line
	9950 1100 8100 1100
Connection ~ 6400 1100
Connection ~ 8100 1100
Wire Wire Line
	8100 1100 6400 1100
Wire Wire Line
	9950 3000 9450 3000
Connection ~ 6400 3000
Connection ~ 8100 3000
Wire Wire Line
	8100 3000 7600 3000
Wire Wire Line
	7600 2700 7600 3000
Connection ~ 7600 3000
Wire Wire Line
	7600 3000 6400 3000
Wire Wire Line
	5900 2700 5900 3000
Wire Wire Line
	5900 3000 6400 3000
Wire Wire Line
	9450 2700 9450 3000
Connection ~ 9450 3000
Wire Wire Line
	9450 3000 8100 3000
Text GLabel 5900 2400 0    50   Input ~ 0
INP_LOAD
Text GLabel 5900 2600 0    50   Input ~ 0
INP_CLK
Text GLabel 7600 2600 0    50   Input ~ 0
INP_CLK
Text GLabel 9450 2600 0    50   Input ~ 0
INP_CLK
Text GLabel 7600 2400 0    50   Input ~ 0
INP_LOAD
Text GLabel 9450 2400 0    50   Input ~ 0
INP_LOAD
Text GLabel 4350 2400 2    50   Input ~ 0
INP_LOAD
Text GLabel 4350 2500 2    50   Input ~ 0
INP_CLK
Text GLabel 1600 2600 0    50   Input ~ 0
INP_DATA
Wire Wire Line
	1050 3000 1600 3000
Wire Wire Line
	1050 2500 1600 2500
$Comp
L Device:C C3
U 1 1 61B86FF7
P 9050 5950
F 0 "C3" H 9165 5996 50  0000 L CNN
F 1 "100nF" H 9165 5905 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W2.5mm_P5.00mm" H 9088 5800 50  0001 C CNN
F 3 "~" H 9050 5950 50  0001 C CNN
	1    9050 5950
	1    0    0    -1  
$EndComp
$Comp
L Device:C C4
U 1 1 61B88139
P 9600 5950
F 0 "C4" H 9715 5996 50  0000 L CNN
F 1 "100nF" H 9715 5905 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W2.5mm_P5.00mm" H 9638 5800 50  0001 C CNN
F 3 "~" H 9600 5950 50  0001 C CNN
	1    9600 5950
	1    0    0    -1  
$EndComp
$Comp
L Device:C C5
U 1 1 61B88E26
P 10100 5950
F 0 "C5" H 10215 5996 50  0000 L CNN
F 1 "100nF" H 10215 5905 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W2.5mm_P5.00mm" H 10138 5800 50  0001 C CNN
F 3 "~" H 10100 5950 50  0001 C CNN
	1    10100 5950
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C2
U 1 1 61B89ACF
P 8300 5950
F 0 "C2" H 8418 5996 50  0000 L CNN
F 1 "1500uF" H 8418 5905 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D10.0mm_P2.50mm_P5.00mm" H 8418 5859 50  0001 L CNN
F 3 "~" H 8300 5950 50  0001 C CNN
	1    8300 5950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61B8A8C0
P 8300 6100
F 0 "#PWR?" H 8300 5850 50  0001 C CNN
F 1 "GND" H 8305 5927 50  0000 C CNN
F 2 "" H 8300 6100 50  0001 C CNN
F 3 "" H 8300 6100 50  0001 C CNN
	1    8300 6100
	1    0    0    -1  
$EndComp
$Comp
L Device:C C6
U 1 1 61B8EA0B
P 10600 5950
F 0 "C6" H 10715 5996 50  0000 L CNN
F 1 "100nF" H 10715 5905 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W2.5mm_P5.00mm" H 10638 5800 50  0001 C CNN
F 3 "~" H 10600 5950 50  0001 C CNN
	1    10600 5950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61B8EE8A
P 9050 6100
F 0 "#PWR?" H 9050 5850 50  0001 C CNN
F 1 "GND" H 9055 5927 50  0000 C CNN
F 2 "" H 9050 6100 50  0001 C CNN
F 3 "" H 9050 6100 50  0001 C CNN
	1    9050 6100
	1    0    0    -1  
$EndComp
Wire Wire Line
	10600 6100 10100 6100
Connection ~ 9050 6100
Connection ~ 9600 6100
Wire Wire Line
	9600 6100 9050 6100
Connection ~ 10100 6100
Wire Wire Line
	10100 6100 9600 6100
Wire Wire Line
	10600 5800 10100 5800
Connection ~ 9600 5800
Wire Wire Line
	9600 5800 9050 5800
Connection ~ 10100 5800
Wire Wire Line
	10100 5800 9600 5800
$Comp
L power:+3.3V #PWR?
U 1 1 61B8FFEF
P 9050 5800
F 0 "#PWR?" H 9050 5650 50  0001 C CNN
F 1 "+3.3V" H 9065 5973 50  0000 C CNN
F 2 "" H 9050 5800 50  0001 C CNN
F 3 "" H 9050 5800 50  0001 C CNN
	1    9050 5800
	1    0    0    -1  
$EndComp
Connection ~ 9050 5800
$Comp
L Device:R_Network08 RN1
U 1 1 61B99878
P 5700 1900
F 0 "RN1" V 5083 1900 50  0000 C CNN
F 1 "10K" V 5174 1900 50  0000 C CNN
F 2 "Resistor_THT:R_Array_SIP9" V 6175 1900 50  0001 C CNN
F 3 "http://www.vishay.com/docs/31509/csc.pdf" H 5700 1900 50  0001 C CNN
	1    5700 1900
	0    -1   1    0   
$EndComp
$Comp
L Device:R_Network08 RN2
U 1 1 61B9E6EF
P 7400 1900
F 0 "RN2" V 6783 1900 50  0000 C CNN
F 1 "10K" V 6874 1900 50  0000 C CNN
F 2 "Resistor_THT:R_Array_SIP9" V 7875 1900 50  0001 C CNN
F 3 "http://www.vishay.com/docs/31509/csc.pdf" H 7400 1900 50  0001 C CNN
	1    7400 1900
	0    -1   1    0   
$EndComp
$Comp
L Device:R_Network08 RN3
U 1 1 61BA0A07
P 9250 1900
F 0 "RN3" V 8633 1900 50  0000 C CNN
F 1 "10K" V 8724 1900 50  0000 C CNN
F 2 "Resistor_THT:R_Array_SIP9" V 9725 1900 50  0001 C CNN
F 3 "http://www.vishay.com/docs/31509/csc.pdf" H 9250 1900 50  0001 C CNN
	1    9250 1900
	0    -1   1    0   
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 61BA2D88
P 5500 1500
F 0 "#PWR?" H 5500 1350 50  0001 C CNN
F 1 "+3.3V" H 5515 1673 50  0000 C CNN
F 2 "" H 5500 1500 50  0001 C CNN
F 3 "" H 5500 1500 50  0001 C CNN
	1    5500 1500
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 61BA367A
P 7200 1500
F 0 "#PWR?" H 7200 1350 50  0001 C CNN
F 1 "+3.3V" H 7215 1673 50  0000 C CNN
F 2 "" H 7200 1500 50  0001 C CNN
F 3 "" H 7200 1500 50  0001 C CNN
	1    7200 1500
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 61BA3BF3
P 9050 1500
F 0 "#PWR?" H 9050 1350 50  0001 C CNN
F 1 "+3.3V" H 9065 1673 50  0000 C CNN
F 2 "" H 9050 1500 50  0001 C CNN
F 3 "" H 9050 1500 50  0001 C CNN
	1    9050 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	6900 1400 6900 1200
Wire Wire Line
	6900 1200 7600 1200
Wire Wire Line
	7600 1200 7600 1400
Wire Wire Line
	8600 1400 8600 1200
Wire Wire Line
	8600 1200 9450 1200
Wire Wire Line
	9450 1200 9450 1400
Text GLabel 10450 1400 2    50   Input ~ 0
INP_DATA
$Comp
L power:GND #PWR?
U 1 1 61BA5875
P 6050 950
F 0 "#PWR?" H 6050 700 50  0001 C CNN
F 1 "GND" H 6055 777 50  0000 C CNN
F 2 "" H 6050 950 50  0001 C CNN
F 3 "" H 6050 950 50  0001 C CNN
	1    6050 950 
	1    0    0    -1  
$EndComp
Wire Wire Line
	5900 1400 5900 900 
Wire Wire Line
	5900 900  6050 900 
Wire Wire Line
	6050 900  6050 950 
$Comp
L Connector:Conn_01x06_Female J1
U 1 1 61BE509A
P 800 4600
F 0 "J1" H 800 4200 50  0000 L CNN
F 1 "Left_Joy" H 350 4200 50  0000 L CNN
F 2 "Connector_JST:JST_EH_S6B-EH_1x06_P2.50mm_Horizontal" H 800 4600 50  0001 C CNN
F 3 "~" H 800 4600 50  0001 C CNN
	1    800  4600
	-1   0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 61BE6DEB
P 1100 4250
F 0 "R?" H 1170 4296 50  0000 L CNN
F 1 "180" H 1170 4205 50  0000 L CNN
F 2 "" V 1030 4250 50  0001 C CNN
F 3 "~" H 1100 4250 50  0001 C CNN
	1    1100 4250
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 61BE7A88
P 1400 4250
F 0 "R?" H 1470 4296 50  0000 L CNN
F 1 "220" H 1470 4205 50  0000 L CNN
F 2 "" V 1330 4250 50  0001 C CNN
F 3 "~" H 1400 4250 50  0001 C CNN
	1    1400 4250
	1    0    0    -1  
$EndComp
Wire Wire Line
	1000 4400 1100 4400
Wire Wire Line
	1000 4500 1400 4500
Wire Wire Line
	1400 4500 1400 4400
$Comp
L power:GND #PWR?
U 1 1 61BE9A83
P 950 3950
F 0 "#PWR?" H 950 3700 50  0001 C CNN
F 1 "GND" H 955 3777 50  0000 C CNN
F 2 "" H 950 3950 50  0001 C CNN
F 3 "" H 950 3950 50  0001 C CNN
	1    950  3950
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 61BE9F3D
P 1400 4100
F 0 "#PWR?" H 1400 3950 50  0001 C CNN
F 1 "+3.3V" H 1415 4273 50  0000 C CNN
F 2 "" H 1400 4100 50  0001 C CNN
F 3 "" H 1400 4100 50  0001 C CNN
	1    1400 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1100 4100 1100 3950
Wire Wire Line
	1100 3950 950  3950
Text GLabel 1000 4600 2    50   Input ~ 0
L_JOY_X
Text GLabel 1000 4700 2    50   Input ~ 0
L_JOY_Y
Text GLabel 1000 4800 2    50   Input ~ 0
L_JOY_SW
Text GLabel 1000 4900 2    50   Input ~ 0
L_POT1
$Comp
L Connector:Conn_01x06_Female J2
U 1 1 61BEF27B
P 1950 4600
F 0 "J2" H 1950 4200 50  0000 L CNN
F 1 "Right_Joy" H 1450 4200 50  0000 L CNN
F 2 "Connector_JST:JST_EH_S6B-EH_1x06_P2.50mm_Horizontal" H 1950 4600 50  0001 C CNN
F 3 "~" H 1950 4600 50  0001 C CNN
	1    1950 4600
	-1   0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 61BEF281
P 2250 4250
F 0 "R?" H 2320 4296 50  0000 L CNN
F 1 "180" H 2320 4205 50  0000 L CNN
F 2 "" V 2180 4250 50  0001 C CNN
F 3 "~" H 2250 4250 50  0001 C CNN
	1    2250 4250
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 61BEF287
P 2550 4250
F 0 "R?" H 2620 4296 50  0000 L CNN
F 1 "220" H 2620 4205 50  0000 L CNN
F 2 "" V 2480 4250 50  0001 C CNN
F 3 "~" H 2550 4250 50  0001 C CNN
	1    2550 4250
	1    0    0    -1  
$EndComp
Wire Wire Line
	2150 4400 2250 4400
Wire Wire Line
	2150 4500 2550 4500
Wire Wire Line
	2550 4500 2550 4400
$Comp
L power:GND #PWR?
U 1 1 61BEF290
P 2100 3950
F 0 "#PWR?" H 2100 3700 50  0001 C CNN
F 1 "GND" H 2105 3777 50  0000 C CNN
F 2 "" H 2100 3950 50  0001 C CNN
F 3 "" H 2100 3950 50  0001 C CNN
	1    2100 3950
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 61BEF296
P 2550 4100
F 0 "#PWR?" H 2550 3950 50  0001 C CNN
F 1 "+3.3V" H 2565 4273 50  0000 C CNN
F 2 "" H 2550 4100 50  0001 C CNN
F 3 "" H 2550 4100 50  0001 C CNN
	1    2550 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2250 4100 2250 3950
Wire Wire Line
	2250 3950 2100 3950
Text GLabel 2150 4600 2    50   Input ~ 0
R_JOY_X
Text GLabel 2150 4700 2    50   Input ~ 0
R_JOY_Y
Text GLabel 2150 4800 2    50   Input ~ 0
R_JOY_SW
Text GLabel 2150 4900 2    50   Input ~ 0
R_POT1
$Comp
L Device:C C?
U 1 1 61BF0184
P 1150 5650
F 0 "C?" H 1265 5696 50  0000 L CNN
F 1 "100nF" H 1265 5605 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W2.5mm_P5.00mm" H 1188 5500 50  0001 C CNN
F 3 "~" H 1150 5650 50  0001 C CNN
	1    1150 5650
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 61BF09A9
P 1600 5650
F 0 "C?" H 1715 5696 50  0000 L CNN
F 1 "100nF" H 1715 5605 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W2.5mm_P5.00mm" H 1638 5500 50  0001 C CNN
F 3 "~" H 1600 5650 50  0001 C CNN
	1    1600 5650
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 61BF0F14
P 2050 5650
F 0 "C?" H 2165 5696 50  0000 L CNN
F 1 "100nF" H 2165 5605 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W2.5mm_P5.00mm" H 2088 5500 50  0001 C CNN
F 3 "~" H 2050 5650 50  0001 C CNN
	1    2050 5650
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 61BF155B
P 2500 5650
F 0 "C?" H 2615 5696 50  0000 L CNN
F 1 "100nF" H 2615 5605 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W2.5mm_P5.00mm" H 2538 5500 50  0001 C CNN
F 3 "~" H 2500 5650 50  0001 C CNN
	1    2500 5650
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 61BF1E79
P 2950 5650
F 0 "C?" H 3065 5696 50  0000 L CNN
F 1 "100nF" H 3065 5605 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W2.5mm_P5.00mm" H 2988 5500 50  0001 C CNN
F 3 "~" H 2950 5650 50  0001 C CNN
	1    2950 5650
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 61BF22B7
P 3400 5650
F 0 "C?" H 3515 5696 50  0000 L CNN
F 1 "100nF" H 3515 5605 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W2.5mm_P5.00mm" H 3438 5500 50  0001 C CNN
F 3 "~" H 3400 5650 50  0001 C CNN
	1    3400 5650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61BF290F
P 1150 5800
F 0 "#PWR?" H 1150 5550 50  0001 C CNN
F 1 "GND" H 1155 5627 50  0000 C CNN
F 2 "" H 1150 5800 50  0001 C CNN
F 3 "" H 1150 5800 50  0001 C CNN
	1    1150 5800
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 5800 2950 5800
Connection ~ 1150 5800
Connection ~ 1600 5800
Wire Wire Line
	1600 5800 1150 5800
Connection ~ 2050 5800
Wire Wire Line
	2050 5800 1600 5800
Connection ~ 2500 5800
Wire Wire Line
	2500 5800 2050 5800
Connection ~ 2950 5800
Wire Wire Line
	2950 5800 2500 5800
Text GLabel 1150 5500 1    50   Input ~ 0
L_JOY_X
Text GLabel 1600 5500 1    50   Input ~ 0
L_JOY_Y
Text GLabel 2050 5500 1    50   Input ~ 0
L_POT_0
Text GLabel 2500 5500 1    50   Input ~ 0
R_JOY_X
Text GLabel 2950 5500 1    50   Input ~ 0
R_JOY_Y
Text GLabel 3400 5500 1    50   Input ~ 0
R_POT_0
$Comp
L Connector:Conn_01x04_Female J3
U 1 1 61BF87E8
P 3150 4500
F 0 "J3" H 3200 4200 50  0000 C CNN
F 1 "POT1" H 2900 4200 50  0000 C CNN
F 2 "Connector_JST:JST_EH_S4B-EH_1x04_P2.50mm_Horizontal" H 3150 4500 50  0001 C CNN
F 3 "~" H 3150 4500 50  0001 C CNN
	1    3150 4500
	-1   0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 61BFEA2A
P 3450 4250
F 0 "R?" H 3520 4296 50  0000 L CNN
F 1 "180" H 3520 4205 50  0000 L CNN
F 2 "" V 3380 4250 50  0001 C CNN
F 3 "~" H 3450 4250 50  0001 C CNN
	1    3450 4250
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 61BFEA30
P 3750 4250
F 0 "R?" H 3820 4296 50  0000 L CNN
F 1 "220" H 3820 4205 50  0000 L CNN
F 2 "" V 3680 4250 50  0001 C CNN
F 3 "~" H 3750 4250 50  0001 C CNN
	1    3750 4250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61BFEA36
P 3300 3950
F 0 "#PWR?" H 3300 3700 50  0001 C CNN
F 1 "GND" H 3305 3777 50  0000 C CNN
F 2 "" H 3300 3950 50  0001 C CNN
F 3 "" H 3300 3950 50  0001 C CNN
	1    3300 3950
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 61BFEA3C
P 3750 4100
F 0 "#PWR?" H 3750 3950 50  0001 C CNN
F 1 "+3.3V" H 3765 4273 50  0000 C CNN
F 2 "" H 3750 4100 50  0001 C CNN
F 3 "" H 3750 4100 50  0001 C CNN
	1    3750 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	3450 4100 3450 3950
Wire Wire Line
	3450 3950 3300 3950
Wire Wire Line
	3350 4400 3450 4400
Wire Wire Line
	3350 4700 3750 4700
Wire Wire Line
	3750 4700 3750 4400
Text GLabel 3350 4500 2    50   Input ~ 0
L_POT1
Text GLabel 3350 4600 2    50   Input ~ 0
R_POT1
$Comp
L Connector:Conn_01x04_Female J4
U 1 1 61C043A2
P 3900 6550
F 0 "J4" H 3800 6800 50  0000 C CNN
F 1 "L_ENC1" H 3900 6250 50  0000 C CNN
F 2 "Connector_JST:JST_EH_S4B-EH_1x04_P2.50mm_Horizontal" H 3900 6550 50  0001 C CNN
F 3 "~" H 3900 6550 50  0001 C CNN
	1    3900 6550
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61C053D5
P 4350 6250
F 0 "#PWR?" H 4350 6000 50  0001 C CNN
F 1 "GND" H 4355 6077 50  0000 C CNN
F 2 "" H 4350 6250 50  0001 C CNN
F 3 "" H 4350 6250 50  0001 C CNN
	1    4350 6250
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C1
U 1 1 61BEC2A5
P 7650 5950
F 0 "C1" H 7768 5996 50  0000 L CNN
F 1 "1000uF" H 7768 5905 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D10.0mm_P2.50mm_P5.00mm" H 7768 5859 50  0001 L CNN
F 3 "~" H 7650 5950 50  0001 C CNN
	1    7650 5950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61BEC2AB
P 7650 6100
F 0 "#PWR?" H 7650 5850 50  0001 C CNN
F 1 "GND" H 7655 5927 50  0000 C CNN
F 2 "" H 7650 6100 50  0001 C CNN
F 3 "" H 7650 6100 50  0001 C CNN
	1    7650 6100
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 61BEC2B1
P 7650 5800
F 0 "#PWR?" H 7650 5650 50  0001 C CNN
F 1 "+5V" H 7665 5973 50  0000 C CNN
F 2 "" H 7650 5800 50  0001 C CNN
F 3 "" H 7650 5800 50  0001 C CNN
	1    7650 5800
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 61BED0CD
P 8300 5800
F 0 "#PWR?" H 8300 5650 50  0001 C CNN
F 1 "+3.3V" H 8315 5973 50  0000 C CNN
F 2 "" H 8300 5800 50  0001 C CNN
F 3 "" H 8300 5800 50  0001 C CNN
	1    8300 5800
	1    0    0    -1  
$EndComp
Text Notes 1650 3750 0    50   ~ 0
Modify ESP32-DevKit modulle:\nNew: Remove regulator which will be repalced by external better one.\nOld:\n1) Cut trace to pin 18 CMD (unused pin)\n2) Diconnect USB +5V from Pin 19 (+5V) at diode\n3) Connect USB +5V from diode to pin 18 CMD\n4) Replace voltage regulator with lower dropout (e.g. 7333)
$Comp
L CarstenKiCadLibrary:LionCharger U?
U 1 1 61BEEE2C
P 6650 5000
F 0 "U?" H 6675 5115 50  0000 C CNN
F 1 "LionCharger" H 6675 5024 50  0000 C CNN
F 2 "" H 6650 5000 50  0001 C CNN
F 3 "" H 6650 5000 50  0001 C CNN
	1    6650 5000
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x03_Male J?
U 1 1 61BF0524
P 7100 6000
F 0 "J?" H 7208 6281 50  0000 C CNN
F 1 "Battery" H 7208 6190 50  0000 C CNN
F 2 "" H 7100 6000 50  0001 C CNN
F 3 "~" H 7100 6000 50  0001 C CNN
	1    7100 6000
	-1   0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x02_Female J?
U 1 1 61BF1CD4
P 7550 5050
F 0 "J?" H 7578 5026 50  0000 L CNN
F 1 "Power Switch" H 7300 4900 50  0000 L CNN
F 2 "" H 7550 5050 50  0001 C CNN
F 3 "~" H 7550 5050 50  0001 C CNN
	1    7550 5050
	1    0    0    -1  
$EndComp
Wire Wire Line
	6900 5900 6750 5900
Wire Wire Line
	6750 5900 6750 5750
Wire Wire Line
	7350 5150 7050 5150
$Comp
L power:GND #PWR?
U 1 1 61BFD7C4
P 6200 5300
F 0 "#PWR?" H 6200 5050 50  0001 C CNN
F 1 "GND" H 6205 5127 50  0000 C CNN
F 2 "" H 6200 5300 50  0001 C CNN
F 3 "" H 6200 5300 50  0001 C CNN
	1    6200 5300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61BFDD25
P 7150 5300
F 0 "#PWR?" H 7150 5050 50  0001 C CNN
F 1 "GND" H 7155 5127 50  0000 C CNN
F 2 "" H 7150 5300 50  0001 C CNN
F 3 "" H 7150 5300 50  0001 C CNN
	1    7150 5300
	1    0    0    -1  
$EndComp
Wire Wire Line
	6300 5300 6200 5300
Wire Wire Line
	7050 5300 7150 5300
$Comp
L power:+5V #PWR?
U 1 1 61C021C4
P 6200 5100
F 0 "#PWR?" H 6200 4950 50  0001 C CNN
F 1 "+5V" H 6215 5273 50  0000 C CNN
F 2 "" H 6200 5100 50  0001 C CNN
F 3 "" H 6200 5100 50  0001 C CNN
	1    6200 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	6300 5150 6200 5150
Wire Wire Line
	6200 5150 6200 5100
$Comp
L Regulator_Linear:TC1262-33 U?
U 1 1 61C0B42A
P 8150 4900
F 0 "U?" H 8150 5142 50  0000 C CNN
F 1 "TC1264-33" H 8150 5051 50  0000 C CNN
F 2 "" H 8150 5125 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/21373C.pdf" H 8150 4600 50  0001 C CNN
	1    8150 4900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61C0EDC7
P 8150 5250
F 0 "#PWR?" H 8150 5000 50  0001 C CNN
F 1 "GND" H 8155 5077 50  0000 C CNN
F 2 "" H 8150 5250 50  0001 C CNN
F 3 "" H 8150 5250 50  0001 C CNN
	1    8150 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	8150 5200 8150 5250
Wire Wire Line
	7350 4900 7850 4900
Wire Wire Line
	7350 4900 7350 5050
$Comp
L power:+3.3V #PWR?
U 1 1 61C12793
P 8550 4900
F 0 "#PWR?" H 8550 4750 50  0001 C CNN
F 1 "+3.3V" H 8565 5073 50  0000 C CNN
F 2 "" H 8550 4900 50  0001 C CNN
F 3 "" H 8550 4900 50  0001 C CNN
	1    8550 4900
	1    0    0    -1  
$EndComp
Wire Wire Line
	8450 4900 8550 4900
$Comp
L power:+3V8 #PWR?
U 1 1 61C14D2C
P 7350 4800
F 0 "#PWR?" H 7350 4650 50  0001 C CNN
F 1 "+3V8" H 7365 4973 50  0000 C CNN
F 2 "" H 7350 4800 50  0001 C CNN
F 3 "" H 7350 4800 50  0001 C CNN
	1    7350 4800
	1    0    0    -1  
$EndComp
$Comp
L power:+3V8 #PWR?
U 1 1 61C15C56
P 9900 3450
F 0 "#PWR?" H 9900 3300 50  0001 C CNN
F 1 "+3V8" H 9915 3623 50  0000 C CNN
F 2 "" H 9900 3450 50  0001 C CNN
F 3 "" H 9900 3450 50  0001 C CNN
	1    9900 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	9900 3500 9900 3450
Wire Wire Line
	7350 4900 7350 4800
Connection ~ 7350 4900
Text GLabel 4100 6550 2    50   Input ~ 0
L_ENC1_A
Text GLabel 4100 6650 2    50   Input ~ 0
L_ENC1_B
Text GLabel 4100 6750 2    50   Input ~ 0
L_ENC1_SW
Wire Wire Line
	4100 6450 4100 6250
Wire Wire Line
	4100 6250 4350 6250
$Comp
L Connector:Conn_01x04_Female J?
U 1 1 61C3A867
P 3900 7300
F 0 "J?" H 3800 7550 50  0000 C CNN
F 1 "R_ENC1" H 3900 7000 50  0000 C CNN
F 2 "Connector_JST:JST_EH_S4B-EH_1x04_P2.50mm_Horizontal" H 3900 7300 50  0001 C CNN
F 3 "~" H 3900 7300 50  0001 C CNN
	1    3900 7300
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61C3A86D
P 4350 7000
F 0 "#PWR?" H 4350 6750 50  0001 C CNN
F 1 "GND" H 4355 6827 50  0000 C CNN
F 2 "" H 4350 7000 50  0001 C CNN
F 3 "" H 4350 7000 50  0001 C CNN
	1    4350 7000
	1    0    0    -1  
$EndComp
Text GLabel 4100 7300 2    50   Input ~ 0
R_ENC1_A
Text GLabel 4100 7400 2    50   Input ~ 0
R_ENC1_B
Text GLabel 4100 7500 2    50   Input ~ 0
R_ENC1_SW
Wire Wire Line
	4100 7200 4100 7000
Wire Wire Line
	4100 7000 4350 7000
$Comp
L Connector_Generic:Conn_02x10_Odd_Even J?
U 1 1 61C3C7FC
P 1100 6900
F 0 "J?" H 1100 7400 50  0000 C CNN
F 1 "L_BUT" H 1150 6300 50  0000 C CNN
F 2 "" H 1100 6900 50  0001 C CNN
F 3 "~" H 1100 6900 50  0001 C CNN
	1    1100 6900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61C3E096
P 900 7450
F 0 "#PWR?" H 900 7200 50  0001 C CNN
F 1 "GND" H 905 7277 50  0000 C CNN
F 2 "" H 900 7450 50  0001 C CNN
F 3 "" H 900 7450 50  0001 C CNN
	1    900  7450
	1    0    0    -1  
$EndComp
Wire Wire Line
	900  7450 900  7400
Connection ~ 900  6600
Wire Wire Line
	900  6600 900  6500
Connection ~ 900  6700
Wire Wire Line
	900  6700 900  6600
Connection ~ 900  6800
Wire Wire Line
	900  6800 900  6700
Connection ~ 900  6900
Wire Wire Line
	900  6900 900  6800
Connection ~ 900  7000
Wire Wire Line
	900  7000 900  6900
Connection ~ 900  7100
Wire Wire Line
	900  7100 900  7000
Connection ~ 900  7200
Wire Wire Line
	900  7200 900  7100
Connection ~ 900  7300
Wire Wire Line
	900  7300 900  7200
Connection ~ 900  7400
Wire Wire Line
	900  7400 900  7300
Text GLabel 1400 6500 2    50   Input ~ 0
L_BUT1
Text GLabel 1400 6700 2    50   Input ~ 0
L_BUT2
Text GLabel 1400 6900 2    50   Input ~ 0
L_BUT3
Text GLabel 1400 7100 2    50   Input ~ 0
L_BUT4
Text GLabel 1400 7300 2    50   Input ~ 0
L_SW1_A
Text GLabel 1400 7400 2    50   Input ~ 0
L_SW1_B
$Comp
L Connector_Generic:Conn_02x10_Odd_Even J?
U 1 1 61C47D3B
P 2250 6850
F 0 "J?" H 2250 7350 50  0000 C CNN
F 1 "R_BUT" H 2300 6250 50  0000 C CNN
F 2 "" H 2250 6850 50  0001 C CNN
F 3 "~" H 2250 6850 50  0001 C CNN
	1    2250 6850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61C47D41
P 2050 7400
F 0 "#PWR?" H 2050 7150 50  0001 C CNN
F 1 "GND" H 2055 7227 50  0000 C CNN
F 2 "" H 2050 7400 50  0001 C CNN
F 3 "" H 2050 7400 50  0001 C CNN
	1    2050 7400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2050 7400 2050 7350
Connection ~ 2050 6550
Wire Wire Line
	2050 6550 2050 6450
Connection ~ 2050 6650
Wire Wire Line
	2050 6650 2050 6550
Connection ~ 2050 6750
Wire Wire Line
	2050 6750 2050 6650
Connection ~ 2050 6850
Wire Wire Line
	2050 6850 2050 6750
Connection ~ 2050 6950
Wire Wire Line
	2050 6950 2050 6850
Connection ~ 2050 7050
Wire Wire Line
	2050 7050 2050 6950
Connection ~ 2050 7150
Wire Wire Line
	2050 7150 2050 7050
Connection ~ 2050 7250
Wire Wire Line
	2050 7250 2050 7150
Connection ~ 2050 7350
Wire Wire Line
	2050 7350 2050 7250
Text GLabel 2550 6450 2    50   Input ~ 0
R_BUT1
Text GLabel 2550 6650 2    50   Input ~ 0
R_BUT2
Text GLabel 2550 6850 2    50   Input ~ 0
R_BUT3
Text GLabel 2550 7050 2    50   Input ~ 0
R_BUT4
Text GLabel 2550 7250 2    50   Input ~ 0
R_SW1_A
Text GLabel 2550 7350 2    50   Input ~ 0
R_SW1_B
$Comp
L Connector:Conn_01x07_Female J?
U 1 1 61C4A67B
P 3150 6750
F 0 "J?" H 3150 7150 50  0000 C CNN
F 1 "SWITCH" H 3050 6350 50  0000 C CNN
F 2 "" H 3150 6750 50  0001 C CNN
F 3 "~" H 3150 6750 50  0001 C CNN
	1    3150 6750
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61C4CB4A
P 3500 6250
F 0 "#PWR?" H 3500 6000 50  0001 C CNN
F 1 "GND" H 3505 6077 50  0000 C CNN
F 2 "" H 3500 6250 50  0001 C CNN
F 3 "" H 3500 6250 50  0001 C CNN
	1    3500 6250
	1    0    0    -1  
$EndComp
Wire Wire Line
	3350 6450 3350 6250
Wire Wire Line
	3350 6250 3500 6250
Text GLabel 3350 6550 2    50   Input ~ 0
L_SW2
Text GLabel 3350 6650 2    50   Input ~ 0
L_SW3
Text GLabel 3350 6850 2    50   Input ~ 0
R_SW2
Text GLabel 3350 6950 2    50   Input ~ 0
R_SW3
Text Notes 6900 4500 0    50   ~ 0
Change regulator to lower dropout HT7333-A ?\n
$EndSCHEMATC
