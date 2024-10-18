int productsMenu() {
    short options;
    do {
        printf("Products menu:\n"
                "\t(1) Stocks.\n"
                "\t(2) Find."
                "\t(3) Back");
    }while (options < 1 || options > 3);

    switch (options)
    {
    case 1:
        /* code */
        break;
    case 2:
        break;
    case 3:
    default:
        break;
    }
    
}