import processing.sound.*;
Pulse pulse;

public static class dim {
  public static float edge_x;
  public static float edge_y;
  public static float size_x;
  public static float size_y;
  public static float space_y;
  public static float menu_y;
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
}

void draw() {
  float tmp =0;
  background(204);
  fill( 200,100,0);
  rect(0, 0, width, dim.menu_y);
  
  for (int i=0; i<3; ++i)
  {
    fill( 0, 100, 100);
    tmp = dim.edge_y + (i+1)*dim.space_y;
    rect(dim.edge_x, tmp, dim.size_x, dim.size_y); // Left
    rect(width - dim.edge_x - dim.size_x, tmp, dim.size_x, dim.size_y); // Left
  }

  if (mousePressed) {
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
        generateSound(2*i);
      } else if (mouseX > (width - dim.edge_x - dim.size_x) && mouseX < width - dim.edge_x) {
        rect(width - dim.edge_x - dim.size_x, tmp, dim.size_x, dim.size_y);
        generateSound(2*i+1);
      }
    }
  }
}

void generateSound(int id) {
  int[] freqTable = {150, 300, 450, 600, 750, 900, 1050};
  pulse = new Pulse(this);
  pulse.play();
  pulse.freq(freqTable[id]);
  delay(50);
  pulse.stop(); 
}
