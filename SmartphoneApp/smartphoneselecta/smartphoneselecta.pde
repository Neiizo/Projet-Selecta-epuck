import processing.sound.*;
Pulse pulse;
PImage img;
PImage img1;
PImage img2;
PImage img3;

public static class dim {
  public static float edge_x;
  public static float edge_y;
  public static float size_x;
  public static float size_y;
  public static float space_y;
  public static float menu_y;
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
  
  dim.pressed = 7;
  //insertion image
  size(400, 400);
  img1 = loadImage("snickers.JPG");

  size(400, 400);
  img2 = loadImage("mars.JPG");

  size(400, 400);
  img3 = loadImage("bueno.JPG");

  size(400, 400);
  img = loadImage("jm.JPG");
}

void draw() {
  float tmp =0;
  background(236, 236, 236);
  fill(15, 5, 107);
  rect(0, 0, width, dim.menu_y);

  for (int i=0; i<3; ++i)
  {
    fill( 15, 5, 107);
    tmp = dim.edge_y + (i+1)*dim.space_y;
    rect(dim.edge_x, tmp, dim.size_x, dim.size_y); // Left
    rect(width - dim.edge_x - dim.size_x, tmp, dim.size_x, dim.size_y); // Right
  }
  fill(255, 255, 255);
  textSize(150);
  textAlign(CENTER);
  text("MENU", width/2, 300);

  //bouton de start et reset
  //  text("M", width/(6.5), 1100);
  // 1) 890 2)1400 3)1910 //1.8
  fill(255, 255, 255);
  textSize(100);
  textAlign(LEFT);
  text("START", width/(6.4), 890);

  fill(255, 255, 255);
  textSize(100);
  textAlign(RIGHT);
  text("RESET", width/(1.19), 890);

  image(img1, 175, 1220, width/4, height/8);
  image(img2, 175, 1720, width/4, height/8);
  image(img3, 635, 1220, width/4, height/8);
  image(img, 635, 1720, width/4, height/8);



  if (mousePressed) {
    //noStroke();
    checkPosPress();
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
    generateSound(dim.pressed);
    dim.pressed = 7;
  }
}

void generateSound(int id) {
  int[] freqTable = {150, 300, 450, 600, 750, 900, 1050, 0};
  pulse = new Pulse(this);
  pulse.play();
  pulse.freq(freqTable[id]);
  delay(500);
  pulse.stop();
  
  //ajouter qqch qui bloque tout le programme
}
