
#ifndef LG_MISC_H
#define LG_MISC_H




int freeRam()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


#endif  // #ifndef LG_LORA_H
