import os
os.system("")

print(  "\033[40m Black \033[41m Red \033[42m Green \033[43m Yellow \033[44m Blue \033[45m Purple \033[46m Cyan \033[47m White \n"
        "\033[0;100m Black \033[0;101m Red \033[0;102m Green \033[0;103m Yellow \033[0;104m Blue \033[0;105m Purple \033[0;106m Cyan \033[0;107m White \033[0m")

print()

for i in range(256):

    print(f"\033[48;5;{i}m {i:03} ", end='')

    if (i % 16 == 15):
        print("\033[0m")
