/**************************************************************************
 
 * Port lines used: Data1 to Data4 - P0.23 to P0.26
 * En - P0.28. RS - P0.27, RW - Ground
 * Connection : CND to CNAD. Short jumper JP16
 ***************************************************************************/

#include <lpc17xx.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define RS_CTRL  0x08000000  //P0.27
#define EN_CTRL  0x10000000  //P0.28
#define DT_CTRL  0x07800000  //P0.23 to P0.26 data lines
#define MAX 30

void scan(void);

void lcd_init(void);
void wr_cn(void);
void clr_disp(void);
void delay_lcd(unsigned int);
void lcd_com(void);						   
void wr_dn(void);
void lcd_data(void);
void clear_ports(void);
void lcd_puts(unsigned char *);
void EINT3_IRQHandler(void);

unsigned int count = 0;
unsigned int i,ni;
unsigned int mode = 0;
unsigned char ascii_code[4][4] = {'0','1','2','3','4','5','6','7','8','9','+','-','*','/','C','='};
unsigned char code[4][4]={'s','c','t','^','l','!','e','m','i','.','(',')','b','n','C','0'};
unsigned int row,col,flag;
unsigned long int temp,temp1=0, temp2=0, temp3,temp4;
//unsigned int index = 0;
char msg[32] = {""};
char expr[32] = {""};
unsigned int num1=0,num2=0;
signed int res=0;
unsigned char Msg1[] = {"1"};
unsigned char Msg2[] = {"2"};

void lcd_init()
{
	/* Ports initialized as GPIO */
    LPC_PINCON->PINSEL1 &= 0xFC003FFF;  //P0.23 to P0.28

	/* Setting the directions as output */
    LPC_GPIO0->FIODIR |= DT_CTRL;	
	LPC_GPIO0->FIODIR |= RS_CTRL;	
	LPC_GPIO0->FIODIR |= EN_CTRL;	
        
    clear_ports();
	delay_lcd(3200);

	temp2 = (0x30<<19);
	wr_cn();	   
	delay_lcd(30000); 
		
	temp2 = (0x30<<19);
	wr_cn();
	delay_lcd(30000);	 
		
	temp2 = (0x30<<19);
	wr_cn();
	delay_lcd(30000);

	temp2 = (0x20<<19);
	wr_cn();
	delay_lcd(30000);

	temp1 = 0x28;
	lcd_com();
	delay_lcd(30000);
		
	temp1 = 0x0c;		
	lcd_com();
	delay_lcd(800);
	
	temp1 = 0x06;
	lcd_com();
	delay_lcd(800);
	
	temp1 = 0x01;
	lcd_com();
 	delay_lcd(10000);
	
	temp1 = 0x80;
	lcd_com();
	delay_lcd(800);
    return;
}

void lcd_com(void)
{
	temp2 = temp1 & 0xf0;//move data (26-8+1) times : 26 - HN place, 4 - Bits
	temp2 = temp2 << 19;//data lines from 23 to 26
	wr_cn();
	temp2 = temp1 & 0x0f; //26-4+1
	temp2 = temp2 << 23; 
	wr_cn();
	delay_lcd(1000);
    return;
}

 // command nibble o/p routine
void wr_cn(void)                        //write command reg
{ 	 
	clear_ports();
	LPC_GPIO0->FIOPIN = temp2;		// Assign the value to the data lines    
    LPC_GPIO0->FIOCLR = RS_CTRL;		// clear bit RS
	LPC_GPIO0->FIOSET = EN_CTRL;   	// EN=1
	delay_lcd(25);
	LPC_GPIO0->FIOCLR = EN_CTRL;		 		// EN =0
    return;
    
}

 // data o/p routine which also outputs high nibble first
 // and lower nibble next
void lcd_data(void)
{             
	temp2 = temp1 & 0xf0;
    temp2 = temp2 << 19;
    wr_dn();
    temp2= temp1 & 0x0f;	
    temp2= temp2 << 23;
    wr_dn();
    delay_lcd(1000);	
    return;
} 

 // data nibble o/p routine
void wr_dn(void)
{  	  
	clear_ports();

	LPC_GPIO0->FIOPIN = temp2;		// Assign the value to the data lines    
	LPC_GPIO0->FIOSET = RS_CTRL;	// set bit  RS
	LPC_GPIO0->FIOSET = EN_CTRL;   	// EN=1
	delay_lcd(25);
	LPC_GPIO0->FIOCLR = EN_CTRL;	// EN =0
    return;
}

void delay_lcd(unsigned int r1)
{
  	unsigned int r;
  	for(r=0;r<r1;r++);
    return;
}

void clr_disp(void)
{
	temp1 = 0x01;
	lcd_com();
 	delay_lcd(10000);
    return;
}
void clear_ports(void)
{
    /* Clearing the lines at power on */
	LPC_GPIO0->FIOCLR = DT_CTRL; //Clearing data lines
	LPC_GPIO0->FIOCLR = RS_CTRL;  //Clearing RS line
	LPC_GPIO0->FIOCLR = EN_CTRL; //Clearing Enable line
        
    return;
}

void lcd_puts(unsigned char *buf1)
{
    unsigned int i=0;

    while(buf1[i]!='\0')
    {
        temp1 = buf1[i];
     	lcd_data();
			i++;
        if(i==16)
		{
           	temp1 = 0xc0;
			lcd_com();
		}
         
       }
    return;
}

void EINT3_IRQHandler(void){//Used to change mode
	mode = (mode==0)?1:0;
	if(mode == 0){//Display Mode
        temp1 = 0xCF;//1st message on LCD 1st line
        lcd_com();
        delay_lcd(800);
        lcd_puts(Msg1);
        delay_lcd(5000);
    }else{
        temp1 = 0xCF;//Msg2 on LCD 2nd line
        lcd_com();
        delay_lcd(800);
        lcd_puts(Msg2);
        delay_lcd(5000);
    }
    LPC_GPIOINT->IO0IntClr|=LPC_GPIOINT->IO0IntStatR;//Interrupt clear
}
 
void scan()
{
    temp3 = LPC_GPIO1->FIOPIN;
    temp3 &= 0x07800000;
 
    if(temp3!=0x00)
    {
        flag = 1;
 
        if(temp3 == 0x00800000)        
            col = 0;       
 
        else if(temp3 == 0x01000000)        
            col = 1;       
 
        else if(temp3 == 0x02000000)        
            col = 2;        
 
        else if(temp3 == 0x04000000)        
            col = 3;        
    }
}
//Stack to maintain operands
struct opndstack
{
    int top;
    double items[MAX];
}opndstk;
//Stack to maintain operators
struct optrstack
{
    int top;
    char items[MAX];
}optrstk;
//Push function for operands 
void pushopnd(struct opndstack *s,double val)
{
    if(s->top==MAX-1)
    {
        //printf("\nStack Overflow.\n");
        exit(1);
    }
    else
    {s->items[++(s->top)]=val;
    }
}
//Pop function for operands
double popopnd(struct opndstack *s)
{
    if(s->top==-1)
    {
       // printf("\nStack Underflow.\n");
        exit(1);
    }
    else
    {
    return(s->items[(s->top)--]);
    }
}
//Push function for operators
void pushoptr(struct optrstack *s,char ch)
{
    if(s->top==MAX-1)
    {
        //printf("\nStack Overflow.\n");
        exit(1);
    }
    else
    {   s->items[++(s->top)]=ch;
    }
}
//Pop function for operators
char popoptr(struct optrstack *s)
{
    if(s->top==-1)
    {
    //printf("\nStack Underflow.\n");
    exit(1);
    }
    else
    {
    return(s->items[(s->top)--]);
    }
}
//Check if given character is a digit
int isDigit(char ch)
{   return((ch>='0' && ch<='9')||ch=='.');
    }
//Check if given character is an operator
int isoperator(char ch)
{
    switch(ch)
    {
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
            return 1;
        default:
        return 0;
    }
}
//Return the result after performing operation on operands
double eval(char ch,double opnd1,double opnd2)
{
    switch(ch)
    {
        case '+':return(opnd1+opnd2);
        case '-':return(opnd1-opnd2);
        case '*':return(opnd1*opnd2);
        case '/':return(opnd1/opnd2);
        case '^':return(pow(opnd1,opnd2));
        default://printf("\nInvalid operator.\n");
        exit(1);
    }
}
/*Precedence chart
    1.Bracket evaluation
    2.Power
    3.Multiplication
    4.Addition,Subtraction
*/
int precedence(char ch)
{
    switch(ch)
    {
        case '#': return 0;
        case '+':
        case '-': return 1;
        case '*':
        case '/':return 2;
        case '^':return 3;
        case '(':return 4;
        default ://printf("\nInvalid operator.\n");
        exit(1);
    }
}

//Evaluation of infix expression passed as string
double infix(char str[])
{
    double opnd1,opnd2,value;
    char ch;
    int ni=0;
    char optr2;
    char tempc[30];
	int num;
	if(str[0] == 's' || str[0] == 't' || str[0] == 'c' || str[0] == 'l' || str[0] == '!'
			|| str[0] == 'e' || str[0] == 'm' || str[0] == 'i'||str[0]='b'||str[0]='n'){
		strncpy(tempc, str + 1, strlen(str)-1);
		tempc[strlen(str)]='\0';
		sscanf(tempc, "%d", &num);
		switch(str[0]){
			case 's'://Sine Operation
				return sin(num);
			case 'c'://Cosine Operation
				return cos(num);
			case 't'://Tan Operation
				return tan(num);
			case 'l'://Logarithm Operation
				return log(num);
			case '!'://Factorial
				return fact(num);
			case 'e'://Exponential
				return pow(2.71828,num);
			case 'm'://MOdulus operation
				if(num > 0)
					return num;
				else
					return 0 - num;
			case 'i'://Inverse operation
				return (double) 1/(double)num;
			case 'b'://Covert to binary operation
				return convertToBinary(num);
			case 'n'://Natural logarithm
				return log10(num);
		}
	}
    else{
        int nk=0;
        opndstk.top=-1;
        optrstk.top=-1;
        pushoptr(&optrstk,'#');
        ni=0;
    
	    while(1)
	    {
	        while(isDigit(str[ni])){//Add digits to temp string 
	            tempc[nk++]=str[ni];//while character is a digit
	            ni++;
	        }
	        if(isoperator(str[ni]))
	        {   
	            tempc[nk]='\0';//If char is an operator push temp to operand stack
	            pushopnd(&opndstk,atof(tempc));
	            nk=0;
	            optr2=popoptr(&optrstk);
	            if(precedence(str[ni])>precedence(optr2))//Check precedence order
	                {   pushoptr(&optrstk,optr2);//Push in same order
	                    pushoptr(&optrstk,str[ni]);
	                }
	            else//Push in sorted order
	            {
	                while(precedence(str[ni])<=precedence(optr2))
	                {   opnd2=popopnd(&opndstk);
	                    opnd1=popopnd(&opndstk);
	                    value = eval(optr2,opnd1,opnd2);
	                    pushopnd(&opndstk,value);
	                    optr2=popoptr(&optrstk);
	                }
	                pushoptr(&optrstk,optr2);
	                pushoptr(&optrstk,str[ni]);
	            }
	            ni++;
	        }
	        else if(str[ni]=='\0'){
	            tempc[nk]='\0';
	            pushopnd(&opndstk,atof(tempc));
	            break;
	        }
	    }
	    while((ch=popoptr(&optrstk))!='#')//Evaluate string
	    {
	        opnd2=popopnd(&opndstk);//Get operands
	        opnd1=popopnd(&opndstk);
	        value=eval(ch,opnd1,opnd2);//Operate the required operation
	        pushopnd(&opndstk,value);//Push result back on to stack
	    }	
	    return(popopnd(&opndstk));//Return result
	}
}	

//For factorial
int fact(int num){
    if(num == 1)
	   return 1;
    return num * fact(num -1);
}
//Covert to binary
int convertToBinary(float n) {
	int num = (int)o;
	int binary = 0;
	int count = 0;
	while(num != 0) {
		binary = binary * 10 + num % 2;
		num = num / 2;
		count++;
	}
	
	sol = 0;
	for(z = 0 ; z < count; z++) {
		sol = sol * pow(10, count--) + binary % 10;
		binary = binary / 10;
	}
	num = (int)o;
	if(num % 2 != 0)
		sol = sol + 1;
	return sol;
}


int main(void)
{
    SystemInit();
    SystemCoreClockUpdate();
    
    
    lcd_init();               //initialise LCD
    delay_lcd(3200);
    LPC_GPIO2->FIODIRL = 0x3C00;
    LPC_GPIOINT->IO0IntEnR|=(1<<10);//PO.10 generates interrupt on rising edge
    NVIC_EnableIRQ(EINT3_IRQn);//Enable IO Interrupt Handler:EINT3

    while(1){
        for(row=0;row<4;row++)//Set row as high for scanning
        {
            if(row == 0)
            temp = 0x00000400;
            
            else if(row == 1)
            temp = 0x00000800;
            
            else if(row == 2)
            temp = 0x00001000;
            
            else
                temp = 0x00002000;
            
            LPC_GPIO2->FIOPIN = temp;
            flag = 0;
            scan();
            temp1 = 0x80;
            lcd_com();
            delay_lcd(800);
            
            if(flag==1){//If key is pressed
                if(mode == 0)//If layout 1 
                	temp4 = ascii_code[row][col];//Get ASCII Code for key
                else//If layout 2
                    temp4 = code[row][col];//Get ASCII Code for key
                if(temp4 == 'C'){//Change Mode
                    mode = (mode==0)?1:0;
                    if(mode == 0){//Display Mode
                        temp1 = 0xCF;//1st message on LCD 1st line
                        lcd_com();
                        delay_lcd(800);
                        lcd_puts(Msg1);
                        delay_lcd(5000);
                    }else{
                        temp1 = 0xCF;//Msg2 on LCD 2nd line
                        lcd_com();
                        delay_lcd(800);
                        lcd_puts(Msg2);
                        delay_lcd(5000);
                    }
                    for(i=0;i<50000;i++);
                    clear_ports();
                    
                    temp1 = 0x14;
                    lcd_com();
                    delay_lcd(800);
                    break;
                }else{
                    if(temp4 == '='){//Evaluate expression
                        double d;
                        int m ;
                        char output[15];
                        msg[count++]='=';
                        d = infix(expr);
                        snprintf(output,9,"%f",d);
                        output[10]='\0';
                        for(i=0;i<11;i++)
                    	    msg[count++]=output[i];
                        msg[count]='\0';
                        lcd_puts(msg);
                    }else{//If it is a digit
                        msg[count]=temp4;
                        expr[count]=temp4;
                        count++;
                        msg[count]='\0';
                        expr[count]='\0';
                        lcd_puts(msg);
                    }
                    
                    for(i=0;i<50000;i++);
                    clear_ports();
                    
                    temp1 = 0x14;
                    lcd_com();
                    delay_lcd(800);
                    break;
                }
            }
        }
    }
}
