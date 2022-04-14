#version 410 core

in float Height;

out vec4 FragColor;
//white
// Coords-> 1341,1079
// Color-> (36, 36, 36)
// Color(NDC)-> (0.141, 0.141, 0.141)


//greenish
// Coords-> 900,450
// Color-> (0, 183, 0)
// Color(NDC)-> (0.0, 0.718, 0.0)

// Coords-> 937,609
// Color-> (166, 81, 16)
// Color(NDC)-> (0.651, 0.318, 0.063)


//sand-like
// Coords-> 1015,1073
// Color-> (53, 53, 53)
// Color(NDC)-> (0.208, 0.208, 0.208)

//blue
// Coords-> 756,415
// Color-> (0, 0, 160)
// Color(NDC)-> (0.0, 0.0, 0.627)


void main()
{
   float h = (Height + 16)/64.0f;
   float uh = 1/(h * 10);
   float uuh = 1/(h * 20);
   vec4 color;


   vec4 white = vec4(0.99, 0.99, 0.99 ,  1.0);
   vec4 greenish = vec4(0.0, 0.718, 0.0,  1.0);
   vec4 maroon = vec4(0.8, 0.3, 0.0 , 1.0f);
   vec4 sand_like = vec4(0.7, 0.7, 0.0, 1.0);
   vec4 blue = vec4(0.0, 0.0, 0.627,   1.0);


    if(h < 0.02) {color = blue* vec4(uh , uh, uh , 1.0);}
    else if(h < 0.06) {color = blue;}
    else if(h > 0.06 && h < 0.1) {color = sand_like * vec4(uh , uh, uh , 1.0);}
    else if(h > 0.1 && h < 0.3) {color = greenish * vec4(uh , uh, uh , 1.0);}
    else if(h > 0.3 && h < 0.5) {color = greenish* vec4(uuh , uuh, uuh , 1.0);}
    else if(h > 0.5 && h < 0.6) {color = maroon * vec4 (uh * 1.3 , uh * 1.3, uh * 1.3 , 1.0);}
    else if(h > 0.6 && h < 0.7) {color = maroon * vec4(uh , uh, uh , 1.0);}
    else if(h > 0.7 ){color = white* vec4(h , h, h , 1.0);}

    FragColor = color;
   
   
}