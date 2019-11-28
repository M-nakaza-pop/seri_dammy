/***********************************************************************************************************************/
/* File Name    : 
/* Version      : 
/* Device(s)    :   Ardino nano
/*              :    
/* Description  :   REM >> nano
/* Creation Date: 
/***********************************************************************************************************************/
#include    <MsTimer2.h>
#include    <SPI.h>



/***********************************************************************************************************************/
/*  
/***********************************************************************************************************************/
#define     ldpin       6
#define     clkpin      5
#define     inputpin    3                  /* dammy */

#define     latchpin    7
#define     clockpin    5
#define     outputpin   4
 
#define     DE          8

#define     DS0         14          //A0 PC0
#define     DS1         15          //A1 PC1
#define     DS2         16          //A2 PC2
#define     DS3         17          //A3 PC3


#define PMD2_OUTPUT() (DDRD |= _BV(2))              // pinMode(2,OUTPUT)
#define PMD3_OUTPUT() (DDRD |= _BV(3))              // pinMode(3,OUTPUT)
#define PMD4_OUTPUT() (DDRD |= _BV(4))              // pinMode(4,OUTPUT)
#define PMD5_OUTPUT() (DDRD |= _BV(5))              // pinMode(5,OUTPUT)
#define PMD6_OUTPUT() (DDRD |= _BV(6))              // pinMode(6,OUTPUT)
#define PMD7_OUTPUT() (DDRD |= _BV(7))              // pinMode(7,OUTPUT)

#define PMD8_OUTPUT() (DDRB |= _BV(0))              // pinMode(8,OUTPUT)
#define PMD9_OUTPUT() (DDRB |= _BV(1))              // pinMode(9,OUTPUT)
#define PMD10_OUTPUT() (DDRB |= _BV(2))             // pinMode(10,OUTPUT)
#define PMD11_OUTPUT() (DDRB |= _BV(3))             // pinMode(11,OUTPUT)
#define PMD12_OUTPUT() (DDRB |= _BV(4))             // pinMode(12,OUTPUT)
#define PMD13_OUTPUT() (DDRB |= _BV(5))             // pinMode(13,OUTPUT)
 
#define DWD13_HIGH()   (PORTD |= _BV(13))            // digitalWrite(13,HIGH)
#define DWD13_LOW()    (PORTD &= ~_BV(13))           // digitalWrite(13,LOW)

/***********************************************************************************************************************/
/*   Prototype
/***********************************************************************************************************************/
byte    dip8Read();
byte    dip4Read();
void    startFa();
void    timerFire();
void    remRec();
void    serRead();
void    enqComm();
void    stxComm();
byte    makeLrc(byte* d);
void    convData(); 
void    solCont();
void    iniSpi();
void    outSerial();
void    rockRetry(byte indexnum);
void    inputComm();
void    solRec();
void    testMode();
byte    ascConv(byte numb);
long    hc165Read();
int     select(int key);
void    clkWait();

/***********************************************************************************************************************/
/*  Global variables and functions
/***********************************************************************************************************************/
volatile int state = LOW;
volatile static unsigned long old_ulMicros = 0;
volatile static unsigned long ulMicros = 0;
volatile static unsigned long ulwith = 0;
unsigned long uldata = 0;
static int plus = 0;
volatile static unsigned long ulwait = 0;

byte    data[31];                               //rxdBff
byte    g_index   =   0;                        //bffCount
byte    rxFlg   =   0;

boolean sendwait=   false;
boolean commY   =   false;
boolean remrecv =   false;

byte    dipaddr  =   0x00;
byte    dipmode =   0x01;                       /* test */

byte    outp[16]    ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
byte    instatus[18];                           /* 0=0x02 1.2=FF 3=g 4-7 change 8-11 status 12-15 REM 16=0x03 17=LRC */

unsigned int     inputdata;
unsigned int     lastdata    =0x0000;           /* LastTime DATA */

volatile    int     count1  =   8;
volatile    int     count2  =   1800;

unsigned int     seridata;
byte    openretry   =   0;
byte    closeretry  =   0;

byte    ledout;                                 /* test */

int     testyou;

union {
        unsigned long   rem32;

            struct {                                /* 16bit2個=32bitの構造体 */
                unsigned int     code1;
                unsigned int     comm1;
            } data;
} remin;
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void setup() {
  Serial.begin(19200,SERIAL_8E1);
  
  pinMode(13, OUTPUT);
  pinMode(2, INPUT);                            /* REMCON */
    
  pinMode(DE, OUTPUT);                          /* DE */
  
  pinMode(latchpin, OUTPUT);
  pinMode(clockpin, OUTPUT);
  pinMode(outputpin, OUTPUT);
  //PMD5_OUTPUT();
  //PMD6_OUTPUT();
  //PMD7_OUTPUT();
  pinMode(inputpin,INPUT);                      /* seri input */
  pinMode(ldpin,OUTPUT);                        /* L-LD/H-SHIFT */
  pinMode(clkpin,OUTPUT);

  pinMode(DS0,INPUT);
  pinMode(DS1,INPUT);
  pinMode(DS2,INPUT);
  pinMode(DS3,INPUT);
   
  attachInterrupt(0, blink, RISING);            /* INT0 = pin2 */
  MsTimer2::set(50,timerFire);                  /* 50mSEC */
  MsTimer2::start();
  
  iniSpi();
  dipaddr  = dip8Read();
  dipmode = dip4Read();

  digitalWrite(DE, HIGH);
  testMode();
  
  startFa();

                                            /*  SPI.begin();  */
                                            /*  SPI.setDataMode(SPI_MODE0); */
}
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void loop() 
{
    //testMode();
    serRead();              
    inputComm();
    enqComm();
    stxComm();     
}    
    
/*    rxFlg='!';
    testData();
    stxComm();
    
    while(true){
        testData();             //dammyで置かないとダメ
        outSpi();
    }   
}
 */
 /*   return;
    
    serRead();
    enqComm();
    stxComm();
    outSpi();
    
   */ 

    
     
/*    if (state == HIGH) {
        digitalWrite(13, HIGH);
    }
    else
    {
        digitalWrite(13, LOW);
    }
    serRead();
    chkcomm();
    */
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
byte    dip8Read(){
    
        long    dip8    = 0x0000;
        byte    addr;
        
        dip8    =hc165Read();
        dip8    =hc165Read();
        addr    = ~((byte)(dip8));                  /* NOT */
        return(addr);
}
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
byte    dip4Read(){
        
        byte    mode;

        mode = (~PINC) & 0x0F;
        return(mode);
}

#if 0
void    dip4Read(){
    
        if(0xFF >   analogRead(0)){                 /* DIP SW (ON) = LOW */
            dipmode =   dipmode | 0b00000001;
        }

        if(0xFF >   analogRead(1)){
            dipmode =   dipmode | 0b00000010;
        }
}
#endif
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void    startFa(){
    
        while(true){
            if (0xFA==Serial.read()){
                return;
            } 
            if ( 0x05==Serial.read()){
                
                for(byte num=0  ;num    <30 ; num++){
                    Serial.write(0xFA);    
                }
                return;
            }
        }
}
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void timerFire()
{
    if(count1   !=  0){
        count1--    ;
    }
    if(count2   !=  0){                             /* LIFE COUNT */
        count2--    ;
    }
}
/***********************************************************************************************************************
* Function Name: RemComm 32bitTYPE
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void blink() 
{
        
    ulMicros = micros();
    ulwith = ulMicros - old_ulMicros;
    old_ulMicros = ulMicros;

    if (ulwith > 40000 || ulwith < 400){
        plus = 0;
        uldata = 0;
        state = LOW;
    }
    else{
        if (ulwith > 1500){
        uldata = uldata | 0x80000000 ; 
    }
    if (plus >= 32){
        remrecv =   true ;
        //Serial.print(uldata, HEX);
        remin.rem32   =  uldata;
    }
    else{
        plus++;
        uldata = uldata >> 1;
    }
  }
}
/***********************************************************************************************************************
* Function Name: 
* Description  : バッファリングのみ、処理はenqComm()かstxComm()で
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void serRead()
{
    if(Serial.available())
    {
        data[g_index]   =   Serial.read();
        if(rxFlg    ==0x03)
        {
            rxFlg   ='!';                                     /* 0x21 */
        }
        if(data[g_index]    ==0x02 && rxFlg==0)                 /*スタート検出*/
        { 
            rxFlg   =0x02;
            g_index =0;
            data[g_index]   =0x02;                           /* STX */
        }
        if(data[g_index]    ==0x03 && rxFlg==0x02)              /*エンド検出*/
        { 
            rxFlg=0x03;
        }
        if(data[g_index]    ==0x05 && rxFlg==0)                 /*ENQ検出*/
        {
            rxFlg   ='?';                                    /* 0x3F */
            g_index    =0;
            data[g_index]   =0x05;                           /* ENQ */
        }
        ++g_index;
        g_index &=0x1F;
    }
}
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void enqComm()
{
    if(commY    ==false){
        return;
    }
    
    if(rxFlg=='?' && g_index==3)                            /* serComm()でrxFlg確定 */
    {
        rxFlg=0;
        g_index=0;

        byte convadr =ascConv(data[2]);
        convadr =   ((ascConv(data[1]) << 8)|convadr);      /*端末ADDR ascii2byte --> HEX1byte*/
        
        if(convadr==(dipaddr &0b00111111))                    /* bit7.8はone shot鍵 */
        {
            if(sendwait ==true)                             /*状態変化有り*/
            {
                for(byte index=0;   index<=17;    index++)
                {
                 Serial.write(instatus[index]);             /* instatusはinputComm()で作成済み*/
                }
                
                sendwait   =   false;                       /* input chenge */
            }
            else
            {
                 Serial.write(06);                          /* 状態変化無 ACK SERIAL OUT */   
            }
        }
    }    
}            
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void stxComm()
{
    if(rxFlg=='!')                                              /* serComm()でrxFlg確定 */
    {
        byte convadr =ascConv(data[2]);
        convadr =   ((ascConv(data[1]) << 8)|convadr);          /* ASC->HEX */
        
        if(convadr==(dipaddr &0b00111111))                       /* bit0-5 addr bit7.8はone shot鍵設定 */
        {
            rxFlg   =   0;
            
            if(data[3]=='V')                        /*V電文*/
            {
                String  buff    =   "";
                buff    +=  "FFx00";
                buff    +=  0x00;
                Serial.write(0x02);
                Serial.print(buff);                 /* String なら OK */
                Serial.write(0x03);
                Serial.write(0x7B);
            }
            if(data[3]=='Y')                        /*Y電文*/
            {
                commY   =   true;        
            }
            if(data[3]=='G')                        /*G電文*/
            {
                              
                if(data[13] ==  makeLrc(data))
                {
                    inputArray();                   /* data[4]-[11]並び替え */
                    convData();                     /* data分割 */
                    solCont();
                    count1  =   0;                  /* count1はoutSerial()で間欠動作 0にして即時動作*/
                    Serial.write(06);               /* ACK SERIAL OUT   EOT=0x05 */
                }      
            }
        }
    }
}
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
byte    makeLrc(byte* d)
{
        byte    tmp;
        byte    index   =   1;
        
        tmp =   d[index];
        do{
            index++;
            tmp     =   tmp    ^   d[index];
        }while(data[index]!=0x03);
        
        return(tmp);
}
/***********************************************************************************************************************
* Function Name: 
* Description  : data[]からoutp[]へCopy　ついでにBlinkの加工
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void    convData()           /* conversion data Multiplex data */
{       
        byte    index   =0;
        
        for(byte bufmun    =4;bufmun    <=11;   bufmun++){
            outp[index] =data[bufmun]  &0x0003;
            index++;
            outp[index] =(data[bufmun]   >>2)  &0x0003;
            index++;
        }
        
        for(index   =0;index    <=15;   index++){           /* data[4]-data[11]はoutp[0]-outp[15]に変わった*/
            if(outp[index]  ==0x0003){                      /* 0x03=Blink -> 0x05に置き換え*/
                outp[index] =0x0005;                        /* 0x05->0x04->0x03 この間0.8SEC */
            }
           
        }
}
/***********************************************************************************************************************
* Function Name: ArrayChange
* Description  : data[4]-[11]並び替え
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void    inputArray(){
        
        byte    temp[4];

        temp[0]     =data[4];
        temp[1]     =data[5];
        temp[2]     =data[6]; 
        temp[3]     =data[7];

        data[4]     =temp[2];
        data[5]     =temp[3];
        data[6]     =temp[0];
        data[7]     =temp[1];      
        
        temp[0]     =data[8];
        temp[1]     =data[9];
        temp[2]     =data[10];
        temp[3]     =data[11];

        data[8]     =temp[2];
        data[9]     =temp[3];
        data[10]    =temp[0];
        data[11]    =temp[1];
                     
}
/***********************************************************************************************************************
* Function Name: 
* Description  : outp[14]sol
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void    solCont()
{

        if((dipaddr&0b11000000)!=0b11000000){
            return;
        }
        if(outp[14]==0x00){
            if((inputdata & 0x4000) ==0x0000)            /* 開錠状態 */
            {
                outp[14]    =0x00;                      /* 既に開錠 */
                return;
            }
            else{
                outp[14]    =0x05;                      /* one shot */
                openretry   =5;
                return;
            }
        }
        if(outp[14]==0x01){
            if((inputdata & 0x4000)  ==0x4000)             /* 施錠状態 */
            {
                outp[14]    =0x00;                      /* 既に施錠 */
                return;    
            }
            else{
                outp[14]    =0x05;                      /* one shot */
                closeretry  =5;
                return;
            }
        }
}
/***********************************************************************************************************************
* Function Name: 
* Description  : HC165 CLR
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void    iniSpi()
{
    digitalWrite(clockpin, LOW);            //実行前にlowにする          
    shiftOut(outputpin, clockpin, MSBFIRST, 0x00);            //SPI.transfer(seridata); 
    shiftOut(outputpin, clockpin, MSBFIRST, 0x00);
    
    clkWait();
    clkWait();
    digitalWrite(latchpin, HIGH);
    clkWait();
    digitalWrite(latchpin, LOW);
}
/***********************************************************************************************************************
* Function Name: 
* Description  : outp[] から seridataの作成してseridataをhc595へ出力
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void    outSerial()
{
        if(count1   !=  0)
        {
            return;
        }
        count1  =   8;                                              /*50m*8＝0.4SEC */
      
        unsigned int mask    =0x0001;

        for(byte index  =0; index   <15;   index++)                 /* ASCのOUTは 0-14 */
        {

            rockRetry(index);
            
            switch(outp[index]){
                case    0x00:
                    seridata    =   seridata    &  ~mask;
                    break;
                case    0x01:
                    seridata    =   seridata    |   mask;
                    break;
                case    0x02:                                       /* BLINK */
                    seridata    =   seridata    ^   mask;           /* EOR */
                    break;     
                case    0x03:
                    seridata    =   seridata    &   ~mask;          /* ONE SHOT CLR */ 
                    outp[index] =   0x00;
                    break;

                default:
                    seridata    =   seridata    |   mask;           /* one shot */
                    outp[index]--;                                  /* 0x05->0x04->0x03 この間0.8SEC */  
            }
            mask  =mask    << 1;  
        }
        digitalWrite(clockpin, LOW);            //実行前にはlowにする 
        shiftOut(outputpin, clockpin, MSBFIRST, (byte)seridata);            //SPI.transfer(seridata); 
        shiftOut(outputpin, clockpin, MSBFIRST, (byte)(seridata>>8));
    
        clkWait();
        clkWait();
        digitalWrite(latchpin, HIGH);
        clkWait();
        digitalWrite(latchpin, LOW);
}
/***********************************************************************************************************************
* Function Name: 
* Description  : outp[] から seridataの作成 リレーの為だけに
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void    rockRetry(byte indexnum)
{
        if(indexnum !=14){
            return;
        }
        
        if(outp[14] !=0x00){            /* 14(リレー出力)だけに指定される？ */
            return;            
        }
        
        if((inputdata & 0x4000)==0x4000 && openretry!=0){
            openretry--;
            outp[14]    =0x05;                
        }
        
        if((inputdata & 0x4000)==0x0000 && closeretry!=0){
           closeretry--;
           outp[14]    =0x05; 
        }
}
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void    inputComm(){

        if(sendwait == true){                                   /* 送信待ちならretun */
            return;
        }

        long    shiftdata;      
        int     chengebit;

        inputdata       =(int)hc165Read();
                
        if((lastdata   ^ inputdata)   ==0x0000  &&  remrecv ==   false )    /* 変化が無ければreturn */
        {
            return;
        }
        
        chengebit   =   lastdata   ^ inputdata;                 /* input change */
        lastdata    =   inputdata;
        
        testyou =   chengebit;                                  /* test用*/
           
        for(byte index=4;index<=13;index++){
            instatus[index]   =0x30;                            /* Buff ALL'0'WRITE [14][15]=rem */      
        }
        
        int shiftmask   =0x0001;

        for(byte index=4;index<=7;index++){
            if((chengebit  &shiftmask)  !=0){                  /* 0b00000001 */
                instatus[index]   |=0x01;                      /* 0011 00 01 */
            }
            shiftmask   =shiftmask   <<  1;     
            if((chengebit  &shiftmask)  !=0){                  /* 0b00000010 */
                instatus[index]   |=0x02;                      /* 0011 00 10 */
            }
            shiftmask   =shiftmask   <<  1;
            if((chengebit  &shiftmask)  !=0){                  /* 0b00000100 */
                instatus[index]   |=0x04;                      /* 0011 01 00 */
            }
            shiftmask   =shiftmask   <<  1;
            if((chengebit  &shiftmask)  !=0){                  /* 0b00001000 */
                instatus[index]   |=0x08;                      /* 0011 01 00 */
            }
            shiftmask   =shiftmask  <<  1;
        }
        
            shiftmask   =0x0001;
        
        for(byte index=8;index<=11;index++){
            if((inputdata  &shiftmask) !=0){                  /* 0b00000001 */
                instatus[index]   |=0x01;                       /* 0011 00 01 */
            }
            shiftmask   =shiftmask   <<  1;     
            if((inputdata  &shiftmask)  !=0){                  /* 0b00000010 */
                instatus[index]   |=0x02;                       /* 0011 00 10 */
            }
            shiftmask   =shiftmask   <<  1;
            if((inputdata  &shiftmask)  !=0){                  /* 0b00000100 */
                instatus[index]   |=0x04;                       /* 0011 01 00 */
            }
            shiftmask   =shiftmask   <<  1;
            if((inputdata  &shiftmask)  !=0){                  /* 0b00001000 */
                instatus[index]   |=0x08;                       /* 0011 01 00 */
            }
            shiftmask   =shiftmask  <<  1;
        }

        outputArray();
        
        instatus[0] =0x02;
        instatus[1] =0x46;                                      /* F */
        instatus[2] =0x46;                                      /* F */
        instatus[3] ='g';
        instatus[16] =0x03;                                      /* ETX 挿入 */
       
        solRec();                                               /* [13]      */
        remRec();                                               /* [14] [15] */
        instatus[17]=makeLrc(instatus);
        sendwait = true;
        remrecv  = false;       
}
/***********************************************************************************************************************
* Function Name: ArrayChange
* Description  : instatus[4]-[11]並び替え
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void    outputArray(){
    
        byte    temp[4];

        temp[0]     =instatus[4];
        temp[1]     =instatus[5];
        temp[2]     =instatus[6];
        temp[3]     =instatus[7];
        
        instatus[4] =temp[1];
        instatus[5] =temp[0];
        instatus[6] =temp[7];
        instatus[7] =temp[6];

        temp[0]     =instatus[8];
        temp[1]     =instatus[9];
        temp[2]     =instatus[10];
        temp[3]     =instatus[11];
        
        instatus[8] =temp[1];
        instatus[9] =temp[0];
        instatus[10]=temp[7];
        instatus[11]=temp[6];
        
}
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void    remRec()
{
        if(remrecv  !=true){
            instatus[15]=0x30;                
            instatus[14]=0x30;
            return;
        }                       
 

union {                                                 /* 共用体を使って16bit－8bit変換 */
        unsigned int   conv16;
        /* 8bit2個=16bitの構造体 */
        struct {
            byte    low;
            byte    hi;
        } data;
        } conv;
        
        
        if(remin.data.code1!=0x3581){                   /* 3581 */
            instatus[15]=0x30;                
            instatus[14]=0x30;                       
            return;
        }
        
        conv.conv16   =select(remin.data.comm1) ;
                                  
        instatus[15]=conv.data.low;                   /* 31 */                
        instatus[14]=conv.data.hi;                    /* 30 */
        
        //Serial.print(conv.data.low,HEX);               /* test */
        //Serial.write((char)(remcode16>>8));           /* test */
        
}
/***********************************************************************************************************************
* Function Name: if((dipaddr&0b11000000)!=0b11000000){
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void    solRec(){
    
        if((dipaddr & 0b11000000)==0b10000000){                  /* b7=ON b6=OFF*/
            instatus[13]=0x30;    
        }
        if((dipaddr & 0b11000000)==0b01000000){                  /* b7=OFF b6=ON */
            if(seridata & 0b0100000000000000==0x00){
                instatus[13]=0x31;   
            }
            else{
                instatus[13]=0x30;
            }
        }
}

/***********************************************************************************************************************
* Function Name: 
* Description  : テスト用
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void    ledMoni(){
        
        digitalWrite(13, ledout);
        ledout = !ledout; 
}
/***********************************************************************************************************************
* Function Name: TEST MODE
* Description  : 
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void    testModeOrg()
{
        while(dipmode & 0b00000001){
            if(count1   !=  0)
            {
                return;
            }
                count1  =   8;
            
                int tempdata    =(int)hc165Read();
                digitalWrite(clockpin, LOW);            //実行前にlowにする 
                shiftOut(outputpin, clockpin, MSBFIRST, (byte)tempdata);            //SPI.transfer(seridata); 
                shiftOut(outputpin, clockpin, MSBFIRST, (byte)(tempdata >>8));
    
                clkWait();
                clkWait();
                digitalWrite(latchpin, HIGH);
                clkWait();
                digitalWrite(latchpin, LOW);
        }
}
/***********************************************************************************************************************
* Function Name: TEST MODE
* Description  : 
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void    testMode()
{
    long    txdwait =   2000;
    byte    index   =   0;
    byte    buffer[6]   ;
    long    DEwait;
    
        while(dipmode & 0b00000001){
            if(count1  ==  0){
                
                count1  =   8;
            
                int tempdata    =((int)hc165Read() & 0x3FFF);
                
                if( remrecv==true && remin.data.code1 == 0x3581){                   /* 3581 */
                    bitRead(tempdata,14)==0 ? bitSet(tempdata,14): bitClear(tempdata,14);
                    remrecv  = false; 
                }
                
                digitalWrite(clockpin, LOW);            //実行前にlowにする 
                shiftOut(outputpin, clockpin, MSBFIRST, (byte)tempdata);            //SPI.transfer(seridata); 
                shiftOut(outputpin, clockpin, MSBFIRST, (byte)(tempdata >>8));
    
                clkWait();
                clkWait();
                digitalWrite(latchpin, HIGH);
                clkWait();
                digitalWrite(latchpin, LOW);

            }

            if(--txdwait  ==  0){
                    index   =   0;
                    txdwait =   400000;
                    digitalWrite(DE, LOW);
                    DEwait = millis();
                    Serial.print("test");
                    Serial.flush();
                    digitalWrite(DE, HIGH);
                }

            
            if(Serial.available() > 0){
                buffer[index] = Serial.read();
                index++ ;
            }
            if(index    ==  4){
                 buffer[index] = '\0';
                 String s   =  buffer ;
                    //Serial.print(s);
                    index   =   0;
                 if(s.equals("test")){
                    //Serial.print("ok");
                    txdwait =   2000;
                 }
            }     
            clkWait();   
        }
}
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void    testRead()
{       
        if(count1   !=  0)
        {
            return;
        }
        count1  =   8;
        ledMoni();

        union   hc165data{
            byte    data01;
            int     data02;
        };

        union hc165data shift;
        
        shift.data02    =hc165Read();
        //shift.data02    =shift.data02   >> 1;           /* adjst */
        
        Serial.write(shift.data01);                   /* OK */
        //Serial.write((char)(shift.data02>>8));        /* OK */
}
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
byte    ascConv(byte numb)
{               
        if (numb    <= 0x39)
        {
            return(numb &0x0F);             /* 00-09 */
        }
        if (numb    <=0x46)
        {
            return(numb -0x37);             /* 0A-0F　*/
        }
            return(0x7F);                   /*　ERR  */ 
}       
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
long     hc165Read()
{
        long     hc165data  =0;
        
        digitalWrite(clkpin,LOW);
        digitalWrite(ldpin,LOW);
        clkWait();
        digitalWrite(ldpin,HIGH);
        clkWait();
        if(digitalRead(inputpin)==HIGH){
            hc165data  |=0x0001;
        }
        hc165data    = hc165data << 1;
        
        clkWait();
        
        for(byte loop165=0; loop165<=14 ;loop165++)
        {    
            digitalWrite(clkpin,HIGH);
            clkWait();
            digitalWrite(clkpin,LOW);
            if(digitalRead(inputpin)==HIGH){
            hc165data  |=0x0001;
            }
        hc165data    = hc165data << 1;
        clkWait();    
        }
        return(~(hc165data >>1));                   /* adjst */
}
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
struct  remcode{
        int key ;
        int rnumb   ;
}
const   remcode[41] ={  0xFF00,0x3031,
                        0xFE01,0x3036,
                        0xFD02,0x3042,
                        0xFC03,0x3130,
                        0xFB04,0x3032,
                        0xFA05,0x3037,
                        0xF906,0x3043,          /* "0C", */
                        0xF807,0x3131,          /* "11", */
                        0xF708,0x3033,          /* "03", */
                        0xF609,0x3038,          /* "08", */
                        0xF50A,0x3044,          /* "0D", */
                        0xF40B,0x3132,          /* "12", */
                        0xF30C,0x3034,          /* "04", */
                        0xF20D,0x3039,          /* "09", */
                        0xF10E,0x3045,          /* "0E", */
                        0xF00F,0x3133,          /* "13", */
                        0xEF10,0x3035,          // "05",
                        0xEE11,0x3041,          // "0A",
                        0xED12,0x3046,          // "0F",
                        0xEC13,0x3134,          // "14",
                        0xBF40,0x3135,          // "15",
                        0xBE41,0x3141,          // "1A",
                        0xBD42,0x3146,          // "1F",
                        0xBC43,0x3234,          // "24",
                        0xBB44,0x3136,          // "16",
                        0xBA45,0x3142,          // "1B",
                        0xB946,0x3230,          // "20",
                        0xB847,0x3235,          // "25",
                        0xB748,0x3137,          // "17",
                        0xB649,0x3143,          // "1C",
                        0xB54A,0x3231,          // "21",
                        0xB44B,0x3236,          // "26",
                        0xB34C,0x3138,          // "18",
                        0xB24D,0x3144,          // "1D",
                        0xB14E,0x3232,          // "22",
                        0xB04F,0x3237,          // "27",
                        0xAF50,0x3139,          // "19",
                        0xAE51,0x3145,          // "1E",
                        0xAD52,0x3233,          // "23",
                        0xAC53,0x3238,          // "28",
                        0xACAC,0x3030};         // "00"};

/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
int     select(int key){
        int retnumb;

        for(byte i=0;i<41;i++){
            if(remcode[i].key   ==  key){
                retnumb =remcode[i].rnumb;
                break;
            }
        }
        return(retnumb);
}
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void    clkWait(){
         asm volatile(
        
            "nop \n"
            "nop \n"
            "nop \n"
            "nop \n"
            "nop \n"
            "nop \n"
            "nop \n"
            "nop \n"
            "nop \n"
            "nop \n"
        );
}
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/
void    testData(){
    data[0]=0x02;
    data[1]=0x30;           /* 端末ADDR */
    data[2]=0x31;           /* 端末ADDR */
    data[3]=0x47;           /*  G = 47   V = 56    */
    data[4]=0x30;           /* 1 2 */
    data[5]=0x31;           /* 3 4 */
    data[6]=0x30;           /* 5 6 */
    data[7]=0x30;           /* 7 8 */
    data[8]=0x30;           /* 9 10 */
    data[9]=0x30;           /* 11 12 */
    data[10]=0x30;          /* 13 14 */
    data[11]=0x30;          /* 15 16 */
    data[12]=0x03;
    data[13]=0x44;
}
/***********************************************************************************************************************
* Function Name: 
* Description  : 
* Arguments    : Code
* Return Value : None
***********************************************************************************************************************/    
void    testOutp(){
    
    outp[0]=0x00;           /* 1 */
    outp[1]=0x00;           /* 2 */
    outp[2]=0x00;           /* 3 */
    outp[3]=0x00;           /* 4 */
    outp[4]=0x00;
    outp[5]=0x00;
    outp[6]=0x00;
    outp[7]=0x00;
    outp[8]=0x00;
    outp[9]=0x00;
    outp[10]=0x00;
    outp[11]=0x00;
    outp[12]=0x00;
    outp[13]=0x00;
    outp[14]=0x00;
}
    


