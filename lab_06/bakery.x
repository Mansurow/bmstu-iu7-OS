struct BAKERY
{
    int num;
    int pid;
    int result;
};

program BAKERY_PROG
{
    version BAKERY_VER
    {
        struct BAKERY GET_NUMBER(struct BAKERY) = 1; 
        struct BAKERY WAIT_QUEUE(struct BAKERY) = 2;
        struct BAKERY BAKERY_RES(struct BAKERY) = 3;
    } = 1; /* Version number = 1 */
} = 0x20000001;
