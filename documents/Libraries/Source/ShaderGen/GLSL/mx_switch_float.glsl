void mx_switch_float(float _in1, float _in2, float _in3, float _in4, float _in5, float which, out float result)
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
