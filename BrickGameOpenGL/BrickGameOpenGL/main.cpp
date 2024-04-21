
#include <GL/freeglut_std.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <string>

using namespace std;

#define WINDOW_W 800
#define WINDOW_H 400

/// Panel x and y values 
static const int FPS = 60;
float panel_bot = -7.5, panel_position = -25.0, panel_width = 15.0, panel_height = 3.0;

/// Game values 
const int brickAmount = 20;
int health = 1;
int countBricks = 0;
int score = 0;
int play = 2;
int highest_score = 0;

void end_game();

/// Brick values 
struct bricks {
	float x;
	float y;
	bool isAlive = true;
};

bricks bricksArray[brickAmount];

// Ball values
struct _ball
{
	GLfloat radius = 1.0;
	GLfloat X = 0.0;
	GLfloat Y = 0.0;
	GLint forDirection = 1;
	GLfloat old_x = 0.0;
	GLfloat old_y = 0.0;
}ball;

// Ball movement
static GLfloat moveX = 0.2;
static GLfloat moveY = 0.1;

// Adding bricks to bricksArray and printing the bricks
void createBricks()
{
	float brick_x = -20, brick_y = 20, brick_width = 4, brick_height = 3;

	for (int i = 0; i < brickAmount; i++)
	{
		if (brick_x > 35)
		{
			brick_x = -20;
			brick_y = 12;
		}
		bricksArray[i].x = brick_x;
		bricksArray[i].y = brick_y;
		brick_x = brick_x + 6;
	}
	glColor3ub(255, 165, 0);
	glBegin(GL_QUADS);
	for (int i = 0; i < brickAmount; i++)
	{
		if (bricksArray[i].isAlive == true)
		{
			glVertex2f(bricksArray[i].x, bricksArray[i].y);
			glVertex2f(bricksArray[i].x + brick_width, bricksArray[i].y);
			glVertex2f(bricksArray[i].x + brick_width, bricksArray[i].y - brick_height);
			glVertex2f(bricksArray[i].x, bricksArray[i].y - brick_height);
		}
	}
	glEnd();
}

void reshape(GLint w, GLint h)
{
	glViewport(0, 0, w, h);
	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-50.0, 50.0, -50.0 / aspect, 50.0 / aspect, -1.0, 1.0);
}

//Drawin the ball
void draw_circle(float x, float y, float radius) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(x, y, 0.0f);
	static const int circle_points = 100;
	static const float angle = 2.0f * 3.1416f / circle_points;

	glColor3ub(0, 128, 200);
	glBegin(GL_POLYGON);
	double angle1 = 0.0;
	glVertex2d(radius * cos(0.0), radius * sin(0.0));
	int i;
	for (i = 0; i < circle_points; i++)
	{
		glVertex2d(radius * cos(angle1), radius * sin(angle1));
		angle1 += angle;
	}
	glEnd();
	glPopMatrix();
}

// ball movement changing with forDirection value and timer function is a loop because of the glutPostRedisplay
void timer(int v)
{
	glutPostRedisplay();

	if (ball.forDirection == 0)
	{
		ball.Y = ball.Y + moveY;
	}
	else if (ball.forDirection == 1)
	{
		ball.Y = ball.Y - moveY;												//forDirection = 0	To top
	}																			//forDirection = 1	To bottom
	else if (ball.forDirection == 2)											//forDirection = 2	To right bottom
	{																			//forDirection = 3	To left bottom
		ball.X = ball.X + moveX;												//forDirection = 4	To left top
		ball.Y = ball.Y - moveY;												//forDirection = 5	To right top
	}
	else if (ball.forDirection == 3)
	{
		ball.X = ball.X - moveX;
		ball.Y = ball.Y - moveY;
	}
	else if (ball.forDirection == 4)
	{
		ball.X = ball.X - moveX;
		ball.Y = ball.Y + moveY;
	}
	else if (ball.forDirection == 5)
	{
		ball.X = ball.X + moveX;
		ball.Y = ball.Y + moveY;
	}

	glutTimerFunc(400 / FPS, timer, v);
}

// When ball hit the walls and panel ball movement
void ball_direction(void)
{

	if (ball.Y + ball.radius > 25) // top wall
	{
		if (ball.old_x == ball.X) {
			ball.forDirection = 1;
			ball.old_x = ball.X;
			score = score - 5;
		}
		else if (ball.old_x < ball.X)
		{
			ball.forDirection = 2;
			ball.old_y = ball.Y;
			ball.old_x = ball.X;
			score = score - 5;
		}
		else
		{
			ball.forDirection = 3;
			ball.old_y = ball.Y;
			ball.old_x = ball.X;
			score = score - 5;
		}
	}
	else if (ball.X + ball.radius > 50) //right wall
	{
		if (ball.old_y < ball.Y)
		{
			ball.forDirection = 4;
			ball.old_y = ball.Y;
			ball.old_x = ball.X;
			score = score - 5;
		}
		else
		{
			ball.forDirection = 3;
			ball.old_y = ball.Y;
			ball.old_x = ball.X;
			score = score - 5;
		}
	}

	else if (ball.X - ball.radius < -50) //left wall
	{
		if (ball.old_y < ball.Y)
		{
			ball.forDirection = 5;
			ball.old_y = ball.Y;
			ball.old_x = ball.X;
			score = score - 5;
		}
		else
		{
			ball.forDirection = 2;
			ball.old_y = ball.Y;
			ball.old_x = ball.X;
			score = score - 5;
		}
	}

	else if (ball.Y - ball.radius < -22)  // panel
	{
		if (ball.X >= panel_bot && ball.X <= panel_bot +6)		// if the ball hits left side of the panel ball is going the left side
		{
			ball.forDirection = 4;
			ball.old_y = ball.Y;
			ball.old_x = ball.X;
			score = score + 1;
		}
		else if (ball.X > panel_bot +6 && ball.X <= panel_bot +9)		// if the ball hits middle of the panel ball is going to top
		{
			ball.forDirection = 0;
			ball.old_y = ball.Y;
			ball.old_x = ball.X;
			score = score + 1;
		}
		else if (ball.X > panel_bot +9 && ball.X <= panel_bot +panel_width)		// if the ball hits right side of the panel ball is going to right side
		{
			ball.forDirection = 5;
			ball.old_y = ball.Y;
			ball.old_x = ball.X;
			score = score + 1;
		}
		else if( ball.Y < -25)													// if the ball not hit the panel end the game
		{
			health = health - 1;
			play = 0;
			end_game();
		}
	}

}

// When ball hits the brick ball movement and bricks display
void hit_brick(void)
{
	unsigned int i = 0;

	while (i < 20)
	{
		if (bricksArray[i].isAlive == true)
		{
			if (bricksArray[i].y - 3 <= ball.Y + ball.radius && bricksArray[i].y >= ball.Y + ball.radius)				// if the ball getting same height with bricks
			{
				if (bricksArray[i].x <= ball.X + ball.radius && bricksArray[i].x + 4 >= ball.X + ball.radius)			// if the ball getting same width bricks 
				{
					if (ball.old_y < ball.Y && ball.old_x < ball.X)														// if the ball is coming from left bottom
					{
						bricksArray[i].isAlive = false;
						countBricks = countBricks + 1;
						ball.forDirection = 2;
						ball.old_x = ball.X;
						ball.old_y = ball.Y;
						score = score + 10;
					}
					else if (ball.old_y < ball.Y && ball.old_x > ball.X)												// if the ball coming from right bottom
					{
						bricksArray[i].isAlive = false;
						countBricks = countBricks + 1;
						ball.forDirection = 3;
						ball.old_x = ball.X;
						ball.old_y = ball.Y;
						score = score + 10;
					}
					else if (ball.old_y > ball.Y && ball.old_x < ball.X)												// if the ball coming from left top
					{
						bricksArray[i].isAlive = false;
						countBricks = countBricks + 1;
						ball.forDirection = 5;
						ball.old_x = ball.X;
						ball.old_y = ball.Y;
						score = score + 10;
					}
					else if (ball.old_y > ball.Y && ball.old_x > ball.X)												//if the ball coming from right top
					{
						bricksArray[i].isAlive = false;
						countBricks = countBricks + 1;
						ball.forDirection = 4;
						ball.old_x = ball.X;
						ball.old_y = ball.Y;
						score = score + 10;
					}
					else if (ball.old_y > ball.Y && ball.old_x == ball.X)												// if the ball coming from top with same ball.x
					{
						bricksArray[i].isAlive = false;
						countBricks = countBricks + 1;
						ball.forDirection = 0;
						ball.old_x = ball.X;
						ball.old_y = ball.Y;
						score = score + 10;
					}
					else
					{																									// if the ball coming from bottom with same ball.x
						bricksArray[i].isAlive = false;
						countBricks = countBricks + 1;
						ball.forDirection = 1;
						ball.old_x = ball.X;
						ball.old_y = ball.Y;
						score = score + 10;
					}
				}
			}

		}
		i++;
	}
}
// Printing score and high score 
void printScore()
{
	glRasterPos2f(-48, 23);
	stringstream ss;
	ss << score;
	string s = "Score: " + ss.str();
	for (int i = 0; i < s.length(); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, s[i]);
	}

	glRasterPos2f(-48, 21);
	stringstream ss1;
	ss1 << highest_score;
	string s1 = "Highest Score: " + ss1.str();
	for (int i = 0; i < s1.length(); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, s1[i]);
	}
}

// Main Loop
void display()
{
	if (play == 1)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glColor3ub(255, 0, 0);
		glRectf(panel_bot, panel_position, panel_bot + panel_width, panel_position + panel_height);		// printing panel 
		createBricks();

		if (ball.Y + ball.radius > 25 || ball.X - ball.radius < -50 || ball.X + ball.radius > 50 || ball.Y - ball.radius < -22) //Wall coordinates
		{
			ball_direction();
		}
		else if (bricksArray[0].y - 3 <= ball.Y + ball.radius && bricksArray[0].y >= ball.Y + ball.radius)			// Top side of bricks coordinates
		{
			hit_brick();
		}
		else if (bricksArray[11].y - 3 <= ball.Y + ball.radius && bricksArray[11].y >= ball.Y + ball.radius)		// Bottom side of bricks coordinates
		{
			hit_brick();
		}
		else if (countBricks == 20)																					
		{
			play = 0;
			end_game();
		}
		else if (score < 0) 
		{
			play = 0;
			end_game();
		}
		printScore();
		draw_circle(ball.X, ball.Y, ball.radius);
		glFlush();
		glutSwapBuffers();
	}
	else if (play == 2)							// Openning information
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRasterPos2f(-23, 15);
		string s = "Welcome to Brick Braker game lets explain the game rule:";
		for (int i = 0; i < s.length(); i++)
		{
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, s[i]);
		}

		glRasterPos2f(-23, 12);
		string s1 = "You have only 1 health so if you drop do ball you lose the game";
		for (int i = 0; i < s1.length(); i++)
		{
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, s1[i]);
		}

		glRasterPos2f(-23, 9);
		string s2 = "Hiting brick = +10 point            Hiting walls = -5 point             Hiting panel = +1 point";
		for (int i = 0; i < s2.length(); i++)
		{
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, s2[i]);
		}

		glRasterPos2f(-23, 6);
		string s3 = "Press Up key to start the game";
		for (int i = 0; i < s3.length(); i++)
		{
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, s3[i]);
		}
		glFlush();
		glutSwapBuffers();
	}
}

void end_game()
{
	if (health < 1 || score < 0)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glRasterPos2f(-20, 0);
		stringstream ss;
		ss << score;
		string s = "YOU LOSE     Your score : " + ss.str() + "";
		for (int i = 0; i < s.length(); i++)
		{
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, s[i]);
		}

		glRasterPos2f(-15, -5);
		string s1 = "Click UP to start a new game";
		for (int i = 0; i < s1.length(); i++)
		{
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, s1[i]);
		}
		if (score > highest_score)
		{
			highest_score = score;
		}
	}
	else if (countBricks == 20)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glRasterPos2f(-20, 0);
		stringstream ss2;
		ss2 << score;
		string s2 = "YOU WIN   Your Score : " + ss2.str() + "";
		for (int i = 0; i < s2.length(); i++)
		{
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, s2[i]);
		}

		glRasterPos2f(-15, -5);
		string s3 = "Click UP to start a new game";
		for (int i = 0; i < s3.length(); i++)
		{
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, s3[i]);
		}
		if (score > highest_score)
		{
			highest_score = score;
		}
	}
}
// if "->" pressed move right, if "<-" pressed move left, if "space" presed restart the game
void special_keybord(int key, int x, int y)				
{
	switch (key)
	{
	case GLUT_KEY_RIGHT:
		if (panel_bot + panel_width < 50)
		{
			panel_bot = panel_bot + 1.0;
		}
		break;
	case GLUT_KEY_LEFT:
		if (panel_bot > -50)
		{
			panel_bot = panel_bot - 1.0;
			printf("%d\n", health);
		}
		break;
	case GLUT_KEY_UP:
		panel_bot = -7.5;
		panel_position = -25.0;
		panel_width = 15.0;
		panel_height = 3.0;
		health = 1;
		countBricks = 0;
		score = 0;
		ball.old_x = 0.0;
		ball.old_y = 0.0;
		ball.X = 0.0;
		ball.Y = 0.0;
		ball.forDirection = 1;
		for (int i = 0; i < brickAmount; i++)
		{
			bricksArray[i].isAlive = true;

		}
		play = 1;
		break;
	}
}


void keybordFunc(unsigned char key, int x, int y)
{

}
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WINDOW_W, WINDOW_H);
	glutInitWindowPosition(260, 140);
	glutCreateWindow("Brick Game");
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutTimerFunc(100, timer, 0);
	glutSpecialFunc(special_keybord);
	glutKeyboardFunc(keybordFunc);
	glutMainLoop();

}
