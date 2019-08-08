#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <EEPROM.h>

void setOtherLEDs();
void setPhasers(uint8_t phaserArray);
void POST();
void NavLightToggle(bool navLightOn);
void damageNacelles(int damage);
void firePhasers();

long unsigned int timer=0;
long unsigned int lastTimer=0;
unsigned int timerDelay=10;   //milliseconds per run
long unsigned int randomPhaserTimer=random(5000);
long unsigned int randomPhaserTimerBuffer=0;
unsigned int boardMode;     //modes: normal phaser, binary counter, larson scanner, disco

uint8_t otherLEDs=B0000000;    //0,0,0,PNacelle,PCollector/NavLight/SCollector,SNacelle
const uint8_t PNacelle     = 0b00010000;
const uint8_t PCollector   = 0b00001000;
const uint8_t NavLight     = 0b00000100;
const uint8_t SCollector   = 0b00000010;
const uint8_t SNacelle     = 0b00000001;
unsigned short phaserCounter=0;
byte larsonScanner=B00000001;
bool larsonScannerDirection=false;
/**
 * PFM profiles were generated with the following python code:
 *
 * import math
 *
 * sigmoid = lambda x: math.exp(x) / (1.0 + math.exp(x))
 *
 * pulse_profile1 = [ 10000.0*sigmoid(10.0 * (x / 100.0) - 5.0) for x in range(0, 100, 1) ]
 * pulse_profile2 = [ 7000.0*sigmoid(10.0 * (x / 100.0) - 5.0) for x in range(0, 100, 1) ]
 * pulse_profile3 = [ 5000.0*sigmoid(10.0 * (x / 100.0) - 5.0) for x in range(0, 100, 1) ]
 * pulse_profile4 = [ 3500.0*sigmoid(10.0 * (x / 100.0) - 5.0) for x in range(0, 100, 1) ]
 *
 * A few notes about the above:
 *
 * 1.  PFM profiles represent a sequence of pulse periods in microseconds.  LEDs are dimmed
 *     using PFM at 50% DC and the given pulse periods.
 * 2.  sum(pulse_profile1) ~= 500e3, i.e. 500ms
 *     sum(pulse_profile2) ~= 70%*sum(pulse_profile1)
 *     sum(pulse_profile3) ~= 70%*sum(pulse_profile2)
 *     sum(pulse_profile4) ~= 70%*sum(pulse_profile3)
 *
 */
const uint16_t pulse_profile1[] = {
	66.928509242848563,
    73.915413442819712,
    81.625711531598967,
    90.132986528478227,
    99.518018669043258,
    109.8694263059318,
    121.28434984274234,
    133.86917827664777,
    147.74031693273057,
    163.02499371440948,
    179.86209962091556,
    198.40305734077509,
    218.81270936130477,
    241.270214176692,
    265.96993576865862,
    293.12230751356316,
    322.95464698450508,
    355.71189272636178,
    391.65722796764356,
    431.07254941086114,
    474.25873177566785,
    521.53563078417733,
    573.24175898868759,
    629.73356056996511,
    691.3842034334682,
    758.58180021243561,
    831.72696493922376,
    911.22961014856151,
    997.50489119685176,
    1090.9682119561294,
    1192.0292202211754,
    1301.0847436299785,
    1418.5106490048781,
    1544.6526508353472,
    1679.8161486607555,
    1824.2552380635632,
    1978.1611144141821,
    2141.6501695744146,
    2314.7521650098233,
    2497.3989440488244,
    2689.4142136999512,
    2890.5049737499594,
    3100.255188723876,
    3318.1222783183384,
    3543.4369377420467,
    3775.4066879814545,
    4013.1233988754816,
    4255.5748318834085,
    4501.6600268752209,
    4750.2081252106009,
    5000.0,
    5249.7918747893991,
    5498.3399731247791,
    5744.4251681165915,
    5986.8766011245216,
    6224.5933120185464,
    6456.5630622579565,
    6681.87772168166,
    6899.744811276124,
    7109.4950262500388,
    7310.5857863000492,
    7502.601055951176,
    7685.2478349901767,
    7858.3498304255863,
    8021.8388855858184,
    8175.7447619364366,
    8320.1838513392449,
    8455.3473491646528,
    8581.4893509951235,
    8698.91525637002,
    8807.9707797788251,
    8909.0317880438688,
    9002.4951088031485,
    9088.7703898514374,
    9168.2730350607762,
    9241.4181997875639,
    9308.6157965665316,
    9370.2664394300355,
    9426.7582410113118,
    9478.4643692158224,
    9525.7412682243321,
    9568.927450589139,
    9608.3427720323562,
    9644.2881072736382,
    9677.0453530154955,
    9706.877692486436,
    9734.0300642313414,
    9758.7297858233087,
    9781.1872906386961,
    9801.5969426592255,
    9820.1379003790844,
    9836.975006285591,
    9852.25968306727,
    9866.1308217233527,
    9878.7156501572572,
    9890.1305736940685,
    9900.4819813309568,
    9909.8670134715212,
    9918.3742884684016,
    9926.084586557181
};
const uint16_t pulse_profile2[] = {
	46.849956469993991,
    51.740789409973793,
    57.137998072119274,
    63.093090569934752,
    69.662613068330288,
    76.908598414152252,
    84.899044889919637,
    93.70842479365345,
    103.41822185291139,
    114.11749560008664,
    125.90346973464089,
    138.88214013854258,
    153.16889655291334,
    168.88914992368439,
    186.17895503806102,
    205.1856152594942,
    226.06825288915357,
    248.99832490845327,
    274.16005957735047,
    301.75078458760282,
    331.98111224296753,
    365.07494154892419,
    401.26923129208132,
    440.8134923989756,
    483.96894240342772,
    531.007260148705,
    582.20887545745666,
    637.860727103993,
    698.25342383779628,
    763.67774836929061,
    834.42045415482278,
    910.759320540985,
    992.95745430341481,
    1081.2568555847431,
    1175.8713040625289,
    1276.9786666444943,
    1384.7127800899273,
    1499.15511870209,
    1620.3265155068762,
    1748.1792608341771,
    1882.5899495899657,
    2023.3534816249717,
    2170.1786321067134,
    2322.6855948228367,
    2480.4058564194324,
    2642.7846815870184,
    2809.186379212837,
    2978.9023823183861,
    3151.1620188126544,
    3325.1456876474208,
    3500.0,
    3674.8543123525792,
    3848.8379811873456,
    4021.0976176816139,
    4190.8136207871648,
    4357.215318412982,
    4519.5941435805689,
    4677.3144051771615,
    4829.8213678932871,
    4976.646518375027,
    5117.4100504100343,
    5251.8207391658234,
    5379.6734844931234,
    5500.84488129791,
    5615.287219910073,
    5723.0213333555057,
    5824.128695937472,
    5918.7431444152571,
    6007.0425456965859,
    6089.2406794590152,
    6165.5795458451767,
    6236.3222516307087,
    6301.7465761622034,
    6362.1392728960063,
    6417.7911245425439,
    6468.9927398512955,
    6516.0310575965723,
    6559.1865076010245,
    6598.7307687079192,
    6634.9250584510764,
    6668.0188877570326,
    6698.2492154123975,
    6725.8399404226493,
    6751.0016750915465,
    6773.931747110847,
    6794.8143847405054,
    6813.8210449619392,
    6831.1108500763157,
    6846.8311034470871,
    6861.1178598614579,
    6874.0965302653594,
    6885.8825043999141,
    6896.5817781470887,
    6906.2915752063464,
    6915.10095511008,
    6923.091401585848,
    6930.3373869316692,
    6936.9069094300648,
    6942.8620019278806,
    6948.2592105900258
};
const uint16_t pulse_profile3[] = {
	33.464254621424281,
    36.957706721409856,
    40.812855765799483,
    45.066493264239114,
    49.759009334521629,
    54.9347131529659,
    60.642174921371172,
    66.934589138323886,
    73.870158466365282,
    81.512496857204738,
    89.931049810457779,
    99.201528670387546,
    109.40635468065239,
    120.635107088346,
    132.98496788432931,
    146.56115375678158,
    161.47732349225254,
    177.85594636318089,
    195.82861398382178,
    215.53627470543057,
    237.12936588783393,
    260.76781539208866,
    286.6208794943438,
    314.86678028498255,
    345.6921017167341,
    379.2909001062178,
    415.86348246961188,
    455.61480507428075,
    498.75244559842588,
    545.48410597806469,
    596.01461011058768,
    650.54237181498922,
    709.25532450243907,
    772.32632541767362,
    839.90807433037776,
    912.12761903178159,
    989.080557207091,
    1070.8250847872073,
    1157.3760825049117,
    1248.6994720244122,
    1344.7071068499756,
    1445.2524868749797,
    1550.127594361938,
    1659.0611391591692,
    1771.7184688710233,
    1887.7033439907273,
    2006.5616994377408,
    2127.7874159417042,
    2250.8300134376104,
    2375.1040626053004,
    2500.0,
    2624.8959373946996,
    2749.1699865623896,
    2872.2125840582958,
    2993.4383005622608,
    3112.2966560092732,
    3228.2815311289783,
    3340.93886084083,
    3449.872405638062,
    3554.7475131250194,
    3655.2928931500246,
    3751.300527975588,
    3842.6239174950883,
    3929.1749152127932,
    4010.9194427929092,
    4087.8723809682183,
    4160.0919256696225,
    4227.6736745823264,
    4290.7446754975617,
    4349.45762818501,
    4403.9853898894125,
    4454.5158940219344,
    4501.2475544015742,
    4544.3851949257187,
    4584.1365175303881,
    4620.709099893782,
    4654.3078982832658,
    4685.1332197150177,
    4713.3791205056559,
    4739.2321846079112,
    4762.870634112166,
    4784.4637252945695,
    4804.1713860161781,
    4822.1440536368191,
    4838.5226765077477,
    4853.438846243218,
    4867.0150321156707,
    4879.3648929116544,
    4890.5936453193481,
    4900.7984713296128,
    4910.0689501895422,
    4918.4875031427955,
    4926.129841533635,
    4933.0654108616764,
    4939.3578250786286,
    4945.0652868470343,
    4950.2409906654784,
    4954.9335067357606,
    4959.1871442342008,
    4963.0422932785905
};
const uint16_t pulse_profile4[] = {
	23.424978234996995,
    25.870394704986897,
    28.568999036059637,
    31.546545284967376,
    34.831306534165144,
    38.454299207076126,
    42.449522444959818,
    46.854212396826725,
    51.709110926455693,
    57.058747800043321,
    62.951734867320447,
    69.441070069271291,
    76.584448276456669,
    84.444574961842193,
    93.08947751903051,
    102.5928076297471,
    113.03412644457679,
    124.49916245422664,
    137.08002978867523,
    150.87539229380141,
    165.99055612148376,
    182.53747077446209,
    200.63461564604066,
    220.4067461994878,
    241.98447120171386,
    265.50363007435249,
    291.10443772872833,
    318.93036355199649,
    349.12671191889814,
    381.83887418464531,
    417.21022707741139,
    455.37966027049248,
    496.4787271517074,
    540.62842779237155,
    587.93565203126445,
    638.48933332224715,
    692.35639004496363,
    749.577559351045,
    810.16325775343807,
    874.08963041708853,
    941.29497479498286,
    1011.6767408124858,
    1085.0893160533567,
    1161.3427974114184,
    1240.2029282097162,
    1321.3923407935092,
    1404.5931896064185,
    1489.4511911591931,
    1575.5810094063272,
    1662.5728438237104,
    1750.0,
    1837.4271561762896,
    1924.4189905936728,
    2010.5488088408069,
    2095.4068103935824,
    2178.607659206491,
    2259.7970717902845,
    2338.6572025885807,
    2414.9106839466435,
    2488.3232591875135,
    2558.7050252050171,
    2625.9103695829117,
    2689.8367422465617,
    2750.4224406489552,
    2807.6436099550365,
    2861.5106666777529,
    2912.064347968736,
    2959.3715722076286,
    3003.5212728482929,
    3044.6203397295076,
    3082.7897729225883,
    3118.1611258153544,
    3150.8732880811017,
    3181.0696364480032,
    3208.895562271272,
    3234.4963699256477,
    3258.0155287982861,
    3279.5932538005122,
    3299.3653843539596,
    3317.4625292255382,
    3334.0094438785163,
    3349.1246077061987,
    3362.9199702113247,
    3375.5008375457733,
    3386.9658735554235,
    3397.4071923702527,
    3406.9105224809696,
    3415.5554250381579,
    3423.4155517235436,
    3430.5589299307289,
    3437.0482651326797,
    3442.941252199957,
    3448.2908890735443,
    3453.1457876031732,
    3457.55047755504,
    3461.545700792924,
    3465.1686934658346,
    3468.4534547150324,
    3471.4310009639403,
    3474.1296052950129
};
void readSetEEPROM(){
    if(EEPROM.read(0)>3){   //if EEPROM is too high, reset back to 0
        EEPROM.write(0,0);
    }
    boardMode=EEPROM.read(0);
    EEPROM.write(0,boardMode+1);
    //setPhasers(EEPROM.read(0)+1); //debug
}
// the setup function runs once when you press reset or power the board
void setup() {
    wdt_enable(WDTO_8S);
    delay(50);  //debounce reset button
    readSetEEPROM();    //change mode with reset button
    ADCSRA = 0; //disable ADC to save power
    POST();
    
}

// otherLEDs:
//   bit 4: PNacelle
//   bit 3: PCollector
//   bit 2: NavLight
//   bit 1: SCollector
//   bit 0: SNacelle
void setOtherLEDs() {
    // PNacelle = PD6
    // PCollector = PD5
    // NavLight = PD2
    // SCollector = PB2
    // SNacelle = PB1
	static const uint8_t DDRDMask = 0b01100100;
	static const uint8_t DDRBMask = 0b00000110;
    bool pNacelle = otherLEDs & PNacelle;
    bool pCollector = otherLEDs & PCollector;
    bool navLight = otherLEDs & NavLight;
    bool sCollector = otherLEDs & SCollector;
    bool sNacelle = otherLEDs & SNacelle;
    DDRD = DDRD & ~DDRDMask | (pNacelle << 6) | (pCollector << 5) | (navLight << 2);
    DDRB = DDRB & ~DDRBMask | (sCollector << 2) | (sNacelle << 1);
}

void setPhasers(uint8_t phaserMask) {
    //PHAS1=PC0
    //PHAS2=PC1
    //PHAS3=PC2
    //PHAS4=PC3
    //PHAS5=PD3
    //PHAS6=PD4
    //PHAS7=PB6
    //PHAS8=PB7
	static const uint8_t DDRBMask = 0b11000000;
	static const uint8_t DDRCMask = 0b00001111;
	static const uint8_t DDRDMask = 0b00110000;
    uint8_t DDRBbits = phaserMask & DDRBMask;
    uint8_t DDRCbits = phaserMask & DDRCMask;
    uint8_t DDRDbits = phaserMask & DDRDMask;
    DDRB = DDRB & ~DDRBMask | DDRBbits;
    DDRC = DDRC & ~DDRCMask | DDRCbits;
    DDRD = DDRD & ~(DDRDMask >> 1) | (DDRDbits >> 1);
}
void POST(){
    uint8_t phaserTest=B0000001;
    otherLEDs=B00000001;
    for(int i=0; i<9; i++){
        setPhasers(phaserTest);
        delay(200);
        phaserTest=phaserTest<<1;
    }
    for(int i=0; i<6; i++){
        setOtherLEDs();
        delay(200);
        otherLEDs=otherLEDs<<1;
    }
}
void NavLightToggle(bool navLightOn=false){
    //otherLEDs=otherLEDs^B00000100;
    if(navLightOn){
        otherLEDs |= NavLight;
    }
    else{
        otherLEDs &= ~NavLight;
    }
    setOtherLEDs();
}
void damageNacelles(int damage=0){
    if(damage==0){
        otherLEDs &= NavLight;
    }
    else if(damage==1){
        otherLEDs &= NavLight|SCollector|SNacelle;
    }
    else if (damage==2){
        otherLEDs &= NavLight|PCollector|PNacelle;
    }
    else{
        otherLEDs |= PCollector|PNacelle|SCollector|SNacelle;
    }
    setOtherLEDs();
}

void loop() {
    wdt_reset();
    if(boardMode==0){   //normalMode
        if(millis()-lastTimer>timerDelay){
            lastTimer=millis();
        }
        if(millis()-randomPhaserTimerBuffer>randomPhaserTimer){
            randomPhaserTimerBuffer=millis();
            randomPhaserTimer=random(10000);
            damageNacelles(random(0,20));
            
            firePhasers();
        }
        //otherLEDs = PCollector|PNacelle|SCollector|SNacelle;
        NavLightToggle(1);
    }
    else if(boardMode==1){  //binCounter mode

        setPhasers(phaserCounter);
        otherLEDs=phaserCounter>>8;
        setOtherLEDs();
        delay(100);
        phaserCounter=phaserCounter+1;
    }
    else if(boardMode==2){  //larsonScanner
        otherLEDs=B11111111;
        setOtherLEDs();
        setPhasers(larsonScanner);
        if(larsonScannerDirection==false){
            larsonScanner=larsonScanner<<1;
        }
        else{
            larsonScanner=larsonScanner>>1;
        }
        if(larsonScanner==B10000000 or larsonScanner==B00000001){
            larsonScannerDirection=!larsonScannerDirection;
        }
        delay(200);
    }
    else{       //discoMode
        setPhasers(random(255));
        otherLEDs=random();
        setOtherLEDs();
        delay(random(300,1000));
    }
}

void crossFade(uint8_t start, uint8_t end, const uint16_t pulse_periods_us[], int n_steps) {
	// PFM @start from 100% down to 0% and @end from 0% up to 100%
	for (int i = 0; i < n_steps; i++) {
		setPhasers(start);
		delayMicroseconds(pulse_periods_us[n_steps-i] / 2);
		setPhasers(end);
		delayMicroseconds(pulse_periods_us[i] / 2);
	}
	setPhasers(end);
}

void firePhasers(){
	crossFade(0b00000000, 0b10000001, pulse_profile1, sizeof(pulse_profile1)/sizeof(pulse_profile1[0]));
	crossFade(0b10000001, 0b01000010, pulse_profile2, sizeof(pulse_profile2)/sizeof(pulse_profile2[0]));
	crossFade(0b01000010, 0b00100100, pulse_profile3, sizeof(pulse_profile3)/sizeof(pulse_profile3[0]));
	crossFade(0b00100100, 0b00011000, pulse_profile4, sizeof(pulse_profile4)/sizeof(pulse_profile4[0]));
    if(random(2)==0){
        setPhasers(B00001000);
    }
    else{
        setPhasers(B00010000);
    }
    delay(600);
    setPhasers(0b00000000);
}
