state("midtown2") {
	//int loading_value : 0x1E0CCC;
	int loading_value : 0x2B17C8;
    string256 message_on_screen : 0x22751C, 0xC, 0x20, 0x4, 0x30, 0x10;
}

startup {
    bool split_done = false;
    bool must_split = false;
}

init {
    vars.split_done = false;
}

start { // Start the timer
    if(current.loading_value==1)
        return true;
    return false;
}

isLoading {
    //return current.loading_value != 0;
    //return current.loading_value != 0xFFFFFFFF;
    return current.loading_value != -1;
}

split {
    vars.must_split = false;
    string message = current.message_on_screen;
    
    switch(message){
        case "Good driving!":
        case "Good jumping!":
        case "Great driving!":
        case "You did it!":
        case "You finished 1st!":
        case "You survived the gauntlet!":
        case "You Won!":
        case "You won!":
            if(!vars.split_done){
                vars.must_split = true;
                vars.split_done = true;
            }
            break;
        default:
            if(vars.split_done){
                vars.split_done = false;
            }
            break;
    }
    
    return vars.must_split;
}
