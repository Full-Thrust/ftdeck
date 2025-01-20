/*
*******************************************************************************

 ftdeck v 1.0

 Threshold deck generation program

 History:
 19-Mar-1997 timj
 Coded

*******************************************************************************
*/

/*
*******************
* Declarations
*******************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "gd.h"
#include "gdfontt.h"
#include "gdfonts.h"
#include "gdfontmb.h"
#include "gdfontl.h"
#include "gdfontg.h"

/*
*******************
* Constants
*******************
*/

#define VERSION "1.0"

#ifdef WIN32
#  define SLASH "\\"
#else
#  define SLASH "/"
#endif
#define DECK_SIZE       100
#define NUM_CARD_ROWS   ((DECK_SIZE / 2) + ((DECK_SIZE / 3) * 2) + ((DECK_SIZE / 6) * 3))
#define NUM_FT_SYSTEMS  28
#define NUM_IN_ROW      5
#define MAX_ENTRIES     (NUM_IN_ROW * NUM_CARD_ROWS)
#define MAX_INSTANCES   (MAX_ENTRIES / NUM_FT_SYSTEMS) 
#define MAX_NAME_LEN    20
#define MAX_CLASH       100
#define CARD_W          150
#define CARD_H          200
#define NUM_CARD_IN_ROW 5
#define NUM_CARD_IN_COL 4
#define NUM_CARDS_IN_SHEET (NUM_CARD_IN_ROW * NUM_CARD_IN_COL)
#define NUM_SHEETS      (DECK_SIZE / (NUM_CARD_IN_ROW * NUM_CARD_IN_COL))
#define SHEET_X         (NUM_CARD_IN_ROW * 150)
#define SHEET_Y         (NUM_CARD_IN_COL * CARD_H)
#define SYSTEM_1        "1 System"
#define SYSTEM_2        "2 Systems"
#define SYSTEM_3        "3 Systems"

/*
*******************
* Types
*******************
*/

typedef struct system {
	char name[MAX_NAME_LEN];
	int instances;
	gdImagePtr icon;
} System;


/*
*******************
* Public Global
*******************
*/

System ft_systems[NUM_FT_SYSTEMS];
char * image_dir = NULL;
char * sys_filename = "ft_systems.txt";
char * deck_filename = "ftdeck";
gdImagePtr card_row [NUM_IN_ROW];

int foreground_color=-1;
int background_color=-1;

/* flags
 */
int debug = 0;
int verbose = 0;

/*
*******************
* Functions
*******************
*/

/*
------------------------------------------------------------------------------
Randomize the random number generator from the current time

*/
static void randomize()
{
   time_t the_time;
   struct tm *time_seed;
   int seed;

   the_time = time(NULL);
   time_seed = localtime(&the_time);
   seed = time_seed->tm_sec + time_seed->tm_min + time_seed->tm_hour;
   srand(seed);
}


/*
------------------------------------------------------------------------------
Load the FT system names from a file and load in the corresponding icon gif
files

*/
static void loadSystems()
{	
	int i;
	FILE * sys_file=NULL;
	FILE * icon_file=NULL;
	
	/* open the system definition file and read in the system names and
     * set the instance count to zero
	 */
	sys_file=fopen(sys_filename, "r");	
	if (!sys_file) {
		fprintf(stderr, "*unable to load* %s\n*aborting*\n",sys_filename);
		exit (0);
	}
	for (i=0; i < NUM_FT_SYSTEMS; i++ ) {
		fscanf(sys_file,"%s\n",&ft_systems[i].name);
		ft_systems[i].instances = 0;
	}
	fclose(sys_file);

	/* read in the icon bitmaps for each system, the file name is the same
     * as the system name but with a '.gif' suffix
	 */
	for (i=0; i < NUM_FT_SYSTEMS; i++ ) {
		char icon_filename[256];

		if (image_dir) {
			sprintf(icon_filename,"%s%s%s%s",image_dir,SLASH,
					ft_systems[i].name,".gif");
		}else{
			sprintf(icon_filename,"%s%s",ft_systems[i].name,".gif");
		}
		icon_file = fopen(icon_filename, "rb");
		if (!icon_file) {
			fprintf(stderr,"*unable to load icon* %s\n*aborting*\n",
					icon_filename);
			exit(0);
		}
		ft_systems[i].icon = gdImageCreateFromGif(icon_file);
		if (verbose) {
			printf("loaded icon %s\n",icon_filename);
		}
		fclose(icon_file);
	}
}

/*
------------------------------------------------------------------------------
Read command arguments

*/
void getArgs(int argc, char* argv[])
{
    int c=0;

    while (--argc > 0 && (*++argv)[0] == '-') { /* walks args */
        while (c = *++argv[0]) {                /* walks arg string */
            switch (toupper(c)) {
                case 'D': {
                    debug = 1;
                    break;
                }	
                case 'F': {
                    deck_filename = strdup((++argv)[0]);
                    argc--;
                    break;
                }                
				case 'S': {
                    sys_filename = strdup((++argv)[0]);
                    argc--;
                    break;
                }
                case 'I': {
                    image_dir = strdup((++argv)[0]);
                    argc--;
                    break;
                }
                case 'V': {
                    verbose = 1;
                    break;
                }
                default: {
                    fprintf(stderr,"ftdeck: illegal option %c\n",c);
                    argc=0;
                    break;
                }
            }
            break;
        }
    }
    if (argc) {
        fprintf(stderr,"usage: ftdeck -i image_dir -f deck_filename"
				" -s system_filename\n" );
        exit(1);
    }
}


/*
------------------------------------------------------------------------------
Pick a system at random returning an index to it

*/
static int pickSystem()
{
	return (rand() % NUM_FT_SYSTEMS);
}


/*
------------------------------------------------------------------------------
Check if a system icon is already in a damage row

Return:
 true  - system already in this row
 false - system not already in this row 

*/
int alreadyInRow(gdImagePtr icon,int current) 
{
	int i;
	for (i=0; i < current; i++) {
		if (card_row[i] == icon) {
			return 1;
		}
	}
	return 0;
}


/*
------------------------------------------------------------------------------
Calculate the systems in a damage row, taking into account repeats and
limits on the number of instances in the deck

*/
static getCardRow()
{
	int i;
    int j;
	int idx;
	int max_inst = MAX_INSTANCES;
    int num_clash = 0;
	gdImagePtr icon;

	/* randomly pick systems until we find one which isn't already in the 
     * damage row and which doesn't have too many instances already allocated
     * so we get an even spread of each system type
	 */
	for (i=0; i < NUM_IN_ROW; i++) {
		num_clash = 0;
		idx = pickSystem();
		icon = ft_systems[idx].icon;
		while (alreadyInRow(icon,i) || 
			   ft_systems[idx].instances > max_inst){
			/* already in this row or too many instances pick again
			 */
			if (debug) {
				printf("*clash %s %d\n",ft_systems[idx].name,
					   ft_systems[idx].instances);
			}
			idx = pickSystem();
			icon = ft_systems[idx].icon;
			num_clash++;

			/* if too many clashes we are in an insolvable situation so
			 * increase the limit for this instance only 
			 */
			if (num_clash > MAX_CLASH) {
				max_inst++;
				num_clash = 0;
				if (debug) {
					printf("*bailing out\n");
				}
			}
		}
		/* reset instance limit if increased
		 */
		if (max_inst != MAX_INSTANCES) {
			max_inst = MAX_INSTANCES; 
		}
		card_row[i] = icon;
		ft_systems[idx].instances++;
	}
}


/*
------------------------------------------------------------------------------
Print a row of icons

*/
static printCardRow(gdImagePtr im,int x,int y)
{ 
	int i;
	int ix;
	int w = CARD_W / NUM_IN_ROW;
    
	for (i=0; i < NUM_IN_ROW; i++) {
		ix = x + (w*i) + ((w - card_row[i]->sx) / 2);
		gdImageCopy(im,card_row[i],ix,y,0,0,card_row[i]->sx,card_row[i]->sy);
	}
}


/*
------------------------------------------------------------------------------
Draw the edges for the cards on the sheet

*/
static void drawCardEdges(gdImagePtr im)
{
	int i;

	/* Draw a bounding box around the sheet
	 */
	gdImageLine(im, 0, 0, im->sx-1, 0,foreground_color);
	gdImageLine(im, im->sx-1, 0, im->sx-1, im->sy-1,foreground_color);
	gdImageLine(im, im->sx-1, im->sy-1, 0, im->sy-1,foreground_color);
	gdImageLine(im, 0, im->sy-1, 0 ,0 ,foreground_color);

	/* Draw horizontal lines
	 */
	for (i=0; i < im->sy; i+= CARD_H) {
		gdImageLine(im, 0, i, im->sx-1, i,foreground_color);
	}

	/* Draw vertical lines
	 */
	for (i=0; i < im->sx; i+= CARD_W) {
		gdImageLine(im, i, 0, i, im->sy-1,foreground_color);
	}
	
}

/*
------------------------------------------------------------------------------
Print card title string

*/
static void printTitle(gdImagePtr im, char * title, int x, int y)
{    
	int title_width=0;
    int title_height=0;
	int title_x=x;
	int title_y= y + ( CARD_H / 2) / 2;

	title_height = ((gdFont *) gdFontGiant)->h;
	title_width = ((gdFont *) gdFontGiant)->w * strlen(title);
	title_x += ((CARD_W - title_width) / 2); 
	gdImageString(im, gdFontGiant, title_x, title_y, title, foreground_color);
}    
 

/*
------------------------------------------------------------------------------
Calculate position of the next card


*/
static void nextCard(int *x, int *y)
{
	/* have we reached the end of the row
	 */
	if ( *x + CARD_W >= SHEET_X) {

		/* yes - move onto the next row
		 */
		*y += CARD_H;
		*x = 0;
	}else{

		/* no - move along row 
		 */
		*x += CARD_W;
	}
}


/*
------------------------------------------------------------------------------
Plot the deck to gif files

*/
static void buildDeck()
{
	int i;
    int j;
	int num_one=0;
    int num_two=0;
	int num_three=0;
    int num_cards=0;
    int card_x=0;
	int card_y=0;
	int row_x=0;
	int row_y=0;
	int row_inc = (CARD_H / 2) / 3; 

	FILE * image_file;
	char image_filename[256];
	gdImagePtr im_out=NULL;


	/* for each sheet required open a new image file and create a black 
     * and white image
	 */
	for (i=1; i <= NUM_SHEETS; i++) {
		sprintf(image_filename,"%s%.2d.gif",deck_filename,i);
		image_file=fopen(image_filename, "wb");
    		
		im_out = gdImageCreate(SHEET_X, SHEET_Y);
		background_color = gdImageColorAllocate(im_out, 255, 255, 255);
		foreground_color = gdImageColorAllocate(im_out, 0, 0, 0);

		/* mark up the card edges
		 */
		drawCardEdges(im_out);

		/* draw the cards until we exhaust this sheet
		 */
		card_x = 0;
		card_y = 0;
		for (j=0; j < NUM_CARDS_IN_SHEET; j++) {

			/* draw the cards until we exhaust this sheet
			 */
			row_x = card_x;
			row_y = card_y + (CARD_H / 2);
			if (num_one < DECK_SIZE/2) {

				/* single row card
				 */
				printTitle(im_out,SYSTEM_1,card_x,card_y);				
				getCardRow();
				printCardRow(im_out,row_x,row_y);
				num_one++;

			} else if (num_two < DECK_SIZE/3) {
	   
				/* double row card
				 */
				printTitle(im_out,SYSTEM_2,card_x,card_y);
				getCardRow();
				printCardRow(im_out,row_x,row_y); 
				getCardRow();
				row_y += row_inc;
				printCardRow(im_out,row_x,row_y);	
				num_two++;

			} else {

				/* triple row card
				 */				
				printTitle(im_out,SYSTEM_3,card_x,card_y);
				getCardRow();
				printCardRow(im_out,row_x,row_y); 
				getCardRow();
				row_y += row_inc;
				printCardRow(im_out,row_x,row_y);
				getCardRow();
				row_y += row_inc;
				printCardRow(im_out,row_x,row_y);
				num_three++;
			}
			nextCard(&card_x,&card_y);

		} /*for j*/
		gdImageGif(im_out, image_file);
		fclose(image_file);
		gdImageDestroy(im_out);
	} /*for i*/
}



/*
------------------------------------------------------------------------------
Report on the deck instances

*/
static void report()
{
	int i;

	for (i=0; i<NUM_FT_SYSTEMS; i++) {
		printf("%s %d\n",ft_systems[i].name,ft_systems[i].instances);
	}
}


/*
******************************************************************************
ftdecl main program

******************************************************************************
*/
main (int argc, char* argv[]) 
{   
	/* Get arguments
     */
    getArgs(argc,argv);

    /* Announce
     */
    fprintf(stdout,"ftdeck version %s\n\n",VERSION);

	if (verbose) {
		printf("cards %d\n",DECK_SIZE);
		printf("icons %d\n",MAX_ENTRIES);
		printf("max icon instances %d\n",MAX_INSTANCES);
	}

	/* Load the system definitions
	 */
	loadSystems();

	/* Initialise the random number generator and build the card deck
	 */
	randomize();
	buildDeck();

	if (verbose) {
		report();
	}
}

/******************************************************************************/
