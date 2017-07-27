void mx_switch_vector4(vec4 _in1, vec4 _in2, vec4 _in3, vec4 _in4, vec4 _in5, float which, out vec4 result)
{
    int selection = int(clamp(floor(which), 1, 5));

    if (selection == 1){
        result = _in1;
    } else if (selection == 2){
        result = _in2;
    } else if (selection == 3){
        result = _in3;
    } else if (selection == 4){
        result = _in4;
    } else if (selection == 5){
        result = _in5;
    }
}
