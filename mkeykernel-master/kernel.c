/*
* Copyright (C) 2014  Arjun Sreedharan
* License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html
*/
#include "keyboard_map.h"

/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define ENTER_KEY_CODE 0x1C

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);

/* current cursor location */
unsigned int current_loc = 0;
/* video memory begins at address 0xb8000 */
char *vidptr = (char*)0xb8000;

/**********  MY DEFINITIONS ***********/
unsigned int r1_xpos = 23;
unsigned int r1_ypos = 40;
unsigned int r1_move = 2;

// Bricks position
unsigned int bricks[20][2] = {

{5,20},{5,25},{5,30},{5,35},{5,40},{5,45},{5,50},{5,55},{5,60},{5,65},
{9,20},{9,25},{9,30},{9,35},{9,40},{9,45},{9,50},{9,55},{9,60},{9,65} 
};

// Ball position and movement
unsigned int ball_x = 20;
unsigned int ball_y = 42;
unsigned int ball_move = 1;
unsigned int forDirection = 0;

//Health for game
unsigned int health = 1;
unsigned int count = 0;

//When hit
unsigned int old_x = 20;
unsigned int old_y = 42;


void gotoxy(unsigned int x, unsigned int y);
void draw_strxy(const char *str,unsigned int x, unsigned int y);
void draw_rkt(void);
void clear_rkt(void);
void move_rkt_right(void);

struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];


void idt_init(void)
{
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	/* populate IDT entry of keyboard's interrupt */
	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	/*     Ports
	*	 PIC1	PIC2
	*Command 0x20	0xA0
	*Data	 0x21	0xA1
	*/

	/* ICW1 - begin initialization */
	write_port(0x20 , 0x11);
	write_port(0xA0 , 0x11);

	/* ICW2 - remap offset address of IDT */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	write_port(0x21 , 0x20);
	write_port(0xA1 , 0x28);

	/* ICW3 - setup cascading */
	write_port(0x21 , 0x00);
	write_port(0xA1 , 0x00);

	/* ICW4 - environment info */
	write_port(0x21 , 0x01);
	write_port(0xA1 , 0x01);
	/* Initialization finished */

	/* mask interrupts */
	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}

void kb_init(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(0x21 , 0xFD);
}

void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = 0x07;
	}
}

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
}

void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = 0x07;
	}
}

void gotoxy(unsigned int x, unsigned int y)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = BYTES_FOR_EACH_ELEMENT * (x * COLUMNS_IN_LINE + y);
}

void draw_strxy(const char *str,unsigned int x, unsigned int y)
{
	gotoxy(x,y);
	kprint(str);
}
void draw_rkt(void)
{
	const char *rkt_s = "=====";
	draw_strxy(rkt_s,r1_xpos,r1_ypos);
}

void clear_rkt(void)
{
	const char *rkt_s = "     ";
	draw_strxy(rkt_s,r1_xpos,r1_ypos);
}

void move_rkt_right(void)
{
	clear_rkt();
	r1_ypos = r1_ypos + r1_move;
	draw_rkt();
}
void move_rkt_left(void)
{
	clear_rkt();
	r1_ypos = r1_ypos - r1_move;
	draw_rkt();
}

void clear_brick(void)
{
	const char *cln_brk = "    ";
	draw_strxy(cln_brk, ball_x,ball_y-1);
	draw_strxy(cln_brk, ball_x +1,ball_y-1);
	draw_strxy(cln_brk, ball_x -1,ball_y-1);
}

void draw_bricks(void)
{
	unsigned int i = 0;
	const char *str = "$";
	while(i < 20)
	{
		draw_strxy(str, bricks[i][0] , bricks[i][1]);
		draw_strxy(str, bricks[i][0] , bricks[i][1]+1);
		draw_strxy(str, bricks[i][0]+1 , bricks[i][1]);
		draw_strxy(str, bricks[i][0]+1 , bricks[i][1]+1);

		i++;

	}

}

void draw_background(void)
{
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int i = 0;
	const char *str_top ="-";
	const char *str_left ="|";

	while(i == 0)
	{
		draw_strxy(str_top, LINES-1, y);
		draw_strxy(str_top, x, y);
		y++;

		if(y == COLUMNS_IN_LINE)
		{
			i++;
		}		
	}

	while (i == 1)
	{
		draw_strxy(str_left,x, COLUMNS_IN_LINE-1);
		draw_strxy(str_left,x, y);
		x++;
		if (x == LINES)
		{
			i++;
		}
	}
		
}

void draw_ballxy(const char *str,unsigned int x, unsigned int y)
{
	gotoxy(x,y);
	kprint(str);
}

void draw_ball(void)
{
	const char *ball = "#";
	draw_ballxy(ball, ball_x, ball_y);
}

void clear_ball(void)
{
	const char *str = "  ";
	draw_ballxy(str, ball_x,ball_y);
}

void ball_direction(void)
{

	if(ball_x == 1)
	{
		if(old_y == ball_y){
			forDirection = 1;
			old_x = ball_x;
		}
		else if(old_y < ball_y)
		{
			forDirection = 2;
			old_y = ball_y;
			old_x = ball_x;
		}
		else
		{
			forDirection = 3;
			old_y = ball_y;
			old_x = ball_x;
		}
	}
	else if(ball_y == 2)
	{
		if(old_x < ball_x)
		{
			forDirection = 2;
			old_y = ball_y;
			old_x = ball_x;
		}
		else
		{
			forDirection = 5;
			old_y = ball_y;
			old_x = ball_x;
		}	
	}

	else if(ball_y == 78)
	{
		if(old_x < ball_x)
		{
			forDirection = 3;
			old_y = ball_y;
			old_x = ball_x;
		}
		else
		{
			forDirection = 4;
			old_y = ball_y;
			old_x = ball_x;
		}	
	}

	else if(ball_x == 23)
	{
		if(ball_y == r1_ypos)
		{
			forDirection = 4;
			old_y = ball_y;
			old_x = ball_x;
		}
		else if(ball_y == r1_ypos +1)
		{
			forDirection = 4;
			old_y = ball_y;
			old_x = ball_x;
		}
		else if(ball_y == r1_ypos +2)
		{
			forDirection = 0;
			old_y = ball_y;
			old_x = ball_x;
		}
		else if(ball_y == r1_ypos + 3)
		{
			forDirection = 5;
			old_y = ball_y;
			old_x = ball_x;
		}
		else if(ball_y == r1_ypos +4)
		{
			forDirection = 5;
			old_y = ball_y;
			old_x = ball_x;
		}
		else
		{
			health = health -1;
		}
	}

}
void hit_brick(void)
{
	unsigned int i = 0;
	
	while(i < 20)
	{
		if(bricks[i][0] == ball_x)
		{
			if(bricks[i][1] == ball_y || bricks[i][1]+1 == ball_y)
			{
				if(old_y < ball_y)
				{
					clear_brick();
					bricks[i][0] = 0;
					bricks[i][1] = 0;
					count = count +1;
					forDirection = 5;
					old_x = ball_x;
					old_y = ball_y;
				}
				else if(old_y > ball_y)
				{
					clear_brick();
					bricks[i][0] = 0;
					bricks[i][1] = 0;
					count = count +1;
					forDirection = 4;
					old_x = ball_x;
					old_y = ball_y;								
				}
				else
				{
					clear_brick();
					bricks[i][0] = 0;
					bricks[i][1] = 0;
					count = count +1;
					forDirection = 3;
					old_x = ball_x;
					old_y = ball_y;	
				}
			}
		}
		else if(bricks[i][0]+1 == ball_x)
		{
			if(bricks[i][1] == ball_y || bricks[i][1]+1 == ball_y)
			{
				if(old_y < ball_y)
				{
					clear_brick();
					bricks[i][0] = 0;
					bricks[i][1] = 0;
					count = count +1;
					forDirection = 2;
					old_x = ball_x;
					old_y = ball_y;
				}
				else if(old_y > ball_y)
				{
					clear_brick();
					bricks[i][0] = 0;
					bricks[i][1] = 0;
					count = count +1;
					forDirection = 3;
					old_x = ball_x;
					old_y = ball_y;								
				}
				else
				{
					clear_brick();
					bricks[i][0] = 0;
					bricks[i][1] = 0;
					count = count +1;
					forDirection = 3;
					old_x = ball_x;
					old_y = ball_y;	
				}
			}
				
		}
		i++;
	}
}
void move_handler(void)
{
	if(health == 0)
	{
		clear_screen();
		const char *msg = "YOU LOSE";
		draw_strxy(msg, 12,40);
	}

	else if(count == 20)
	{
		clear_screen();
		const char *msg = "YOU WIN";
		draw_strxy(msg, 12, 40);
	}

	else if(ball_x > bricks[0][0]-1 && ball_x < bricks[11][0]+2)
	{
		hit_brick();
	}
	else if(ball_x == 1 || ball_x == 78 || ball_y == 78 || ball_y == 2 || ball_x == 23)
	{
		ball_direction();
	}

}

void move_ball(void)
{

	//while(count < 20)
	//{
		
		if(forDirection == 0 )												//forDirection = 0  To top same ball_y position
		{																	//forDirection = 1  To bottom same ball_y position
																			//forDirection = 2  To right bottom corner
			clear_ball();													//forDirection = 3  To left bottom corner
			ball_x = ball_x - ball_move ;									//forDirection = 4	To left top corner
			draw_ball();													//forDirection = 5  To right top corner
			
		}
		else if(forDirection == 1)
		{
			clear_ball();
			ball_x = ball_x + ball_move ;
			draw_ball();
			
		}
		else if(forDirection == 2)
		{
			clear_ball();
			ball_x = ball_x + ball_move;
			ball_y = ball_y + ball_move;
			draw_ball();
			
		}
		else if (forDirection == 3)
		{
			clear_ball();
			ball_x = ball_x + ball_move;
			ball_y = ball_y - ball_move;
			draw_ball();
			
		}
		else if (forDirection == 4)
		{
			clear_ball();
			ball_x = ball_x - ball_move;
			ball_y = ball_y - ball_move;
			draw_ball();
			
		}
		else if (forDirection == 5)
		{
			clear_ball();
			ball_x = ball_x - ball_move;
			ball_y = ball_y + ball_move;
			draw_ball();
			
		}
		move_handler();		
		
		/*for(i =0; i <99999; i++)
		{

		}
		for(i =0; i <9999; i++)
		{

		}*/

		
	//}
}

void play(void)
{
	unsigned int i = 0;
	while(i == 0)
	{
		
		for(int i = 0; i <10000000;i++)
		{

		}
		for(int j = 0; j <10000000; j++)
		{

		}


		move_ball();
	}	
}

void keyboard_handler_main(void)
{
	unsigned char status;
	char keycode;

	/* write EOI */
	write_port(0x20, 0x20);

	status = read_port(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		keycode = read_port(KEYBOARD_DATA_PORT);
		if(keycode < 0)
			return;

		if(keycode == ENTER_KEY_CODE) {
			return;
		}
	}

	keycode = keyboard_map[(unsigned char) keycode];
 	switch(keycode)
    	{
    	case 'd':
		case 'D':
			if(r1_ypos<74)
			{
				move_rkt_right();
			}
			break; 	
		case 'a':
		case 'A':
			if(r1_ypos>1)
			{
				move_rkt_left();
			}
			break;
		case 'b':
		case 'B':
			kprint("Stopping the CPU. Bye!");
    			__asm__ __volatile__("hlt");
    	}

}

void kmain(void)
{
	clear_screen();
	idt_init();
	kb_init();
	draw_background();
	draw_rkt();
	draw_bricks();
	draw_ball();
	play();
	

	while(1);
}