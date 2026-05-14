#include <stdio.h>

void graduate(int age){
    if (age >= 18){
        int graduate_year;
        graduate_year = 2025 + 18 - age;
        printf("%d", graduate_year);   
    }
    else {
        printf("You haven't finished your studies");
    }
    printf("\n");
}

void pension(int age){
    if (age < 65){
        int p;
        p = 65 - age;
        printf("%d", p);
    }
    else{
        printf("You already getting a pension");
    }
    printf("\n");
}

void military_service(int age, char gender){
    if (age >= 18 && age <= 30 && gender == 'M'){
        printf("You must service in army");
    }
    else{
        printf("You shouldn't be in the army");
    }
    printf("\n");
}

int main(){
    int age;
    int result1, result2, result3;
    char gender;

    printf("What's your gender?\n");
    scanf("%c", &gender);
    
    printf("Enter your age\n");
    scanf("%d", &age);

    graduate(age);
    pension(age);
    military_service(age, gender);

    return 0;
}