import processing.sound.*;
SinOsc sine;
PImage img1;
PImage img2;
PImage img3;
PImage img;
PImage img0;

boolean lock = false;
boolean unlock = true;
boolean button = false;

int posx = 635;
int posy = 1720;

public static class dim {
  public static float edge_x;
  public static float edge_y;
  public static float size_x;
  public static float size_y;
  public static float space_y;
  public static float menu_y;
  public static float width_rect;
  public static float height_rect;
  public static int pressed;
}

void setup() {
  fullScreen();
  noStroke();
  dim.edge_x= width/8;
  dim.edge_y = height/12;
  dim.size_x = 3*width/9;
  dim.size_y = 2*dim.edge_y;
  dim.space_y = 3*height/13;
  dim.menu_y = 3*height/14;
  //reset button
  dim.width_rect = width-200;
  dim.height_rect = height/6; 
  
  dim.pressed = 7;
  //insertion image
  size(400, 400);
  img1 = loadImage("bueno.JPG");

  size(400, 400);
  img2 = loadImage("mars.JPG");

  size(400, 400);
  img3 = loadImage("snickers.JPG");
  
  // case spéciale 
  size(400, 400);
  img = loadImage("lock.jpg");
  
  size(400,400);
  img0 = loadImage("unlock.JPG");
}

void draw() {
  float tmp =0;
  background(236, 236, 236);
  fill(15, 5, 107);
  rect(0, 0, width, dim.menu_y);

  for (int i=0; i<2; ++i)
  {
    fill( 15, 5, 107);
    tmp = dim.edge_y + (i+2)*dim.space_y;
    rect(dim.edge_x, tmp, dim.size_x, dim.size_y); // Left
    rect(width - dim.edge_x - dim.size_x, tmp, dim.size_x, dim.size_y); // Right
  }
  
  rect(100, 650, dim.width_rect, dim.height_rect);
  
  fill(255, 255, 255);
  textSize(150);
  textAlign(CENTER);
  text("MENU", width/2, 300);

  //bouton de reset
  fill(255, 255, 255);
  textSize(140);
  textAlign(CENTER);
  text("RESET", width/2,870);

  image(img1, 175, 1220, width/4, height/8);
  image(img2, 175, 1720, width/4, height/8);
  image(img3, 635, 1220, width/4, height/8);
  
  // cas spécial :
    if(button)
  {
     lock = true; 
     unlock = false;
  }else if(!button)
  {
     unlock = true;
     lock = false; 
  }
  if(unlock)
  {
    image(img0, 635, 1720, width/4, height/8);
  }else if(lock)
  {
    image(img, 635, 1720, width/4, height/8);
  }
  

  if (mousePressed && dim.pressed == 7) {
    //noStroke();
    checkPosPress();
  }
}


//click sur le carré 4
void mousePressed() {
  if (mouseX > posx && mouseX < posx+ dim.size_x && mouseY > posy && mouseY < posy+dim.size_y) {
    button = !button;
  }  
}

void checkPosPress() {
  float tmp = 0;
  fill(100, 10, 10);
  for (int i=0; i<3; ++i) {
    tmp = dim.edge_y + (i+1)*dim.space_y;
    if (mouseY > tmp && mouseY < tmp + dim.size_y) {
      if (mouseX > dim.edge_x && mouseX < dim.edge_x + dim.size_x) {
        rect(dim.edge_x, tmp, dim.size_x, dim.size_y);
        dim.pressed = 2*i;
      } else if (mouseX > (width - dim.edge_x - dim.size_x) && mouseX < width - dim.edge_x) {
        rect(width - dim.edge_x - dim.size_x, tmp, dim.size_x, dim.size_y);
        dim.pressed = 2*i +1;
      }
    }
  }
}


void mouseReleased()
{
  if(dim.pressed != 7)
  {
    //faire deux cas pour le bouton de toggle :
    if(dim.pressed == 5)
    {
      if(!button)
      {
        generateSound(dim.pressed);
      }else if(button)
      {
        generateSound(dim.pressed+1);
      }
    }
    else
    {
      generateSound(dim.pressed);
    }    
  }
}

void generateSound(int id) {
  int[] freqTable = {2100, 2100, 400, 500, 600, 900, 1100, 0};
  sine = new SinOsc(this);
  sine.play();
  sine.freq(freqTable[id]);
  delay(500);
  sine.stop();
  dim.pressed = 7;  
  
  //ajouter qqch qui bloque tout le programme
}
