#version 330 core

out vec4 FragColor;
// texture sampler
uniform sampler2D TEXTURE1;
uniform int mode;
float lowest_point;
// height mapper
in float Height;

void main()
{ 
    
    float h = (Height + 16)/32.0f;	// shift and scale the height in to a grayscale value
    vec4 color = vec4(h ,h ,h , 1.0f);
    if(h == lowest_point){
          color = vec4(h ,h ,1.0f , 1.0f);
    }
    if(h < lowest_point){
           lowest_point = h;
     }

    if(mode == 0){
        FragColor = mix(texture(TEXTURE1 , vec2(1.0 , 1.0)) , color , 0.3f);}
    if(mode == 1){
        FragColor = vec4(h ,h ,h , 1.0f);
        }
}