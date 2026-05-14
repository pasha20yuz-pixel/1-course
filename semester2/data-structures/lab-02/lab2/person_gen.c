#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// Структура предмета
typedef struct {
    char name[31];                  
    unsigned char lecture_room;      
    unsigned char lab_room;          
    unsigned char hours;             
    enum { EXAM, CREDIT, COURSEWORK } attestation; 
} subject;

// Структура студента
typedef struct {
    char full_name[101];             
    char birth_date[11];              
    char group[21];                   
    subject* subjects;                
    int num_subjects;                 
} student;

// Глобальные переменные для хранения данных
static student* students = NULL;
static int student_count = 0;

// Прототипы функций
void free_students();
int generate_students(int n, int min_d, int max_d);
void print_student(const student* s, int index);
void print_size();
void clean();
int parse_command(char* line);

// Списки для генерации ФИО
static const char* first_names[] = {
    "Иван", "Петр", "Сергей", "Алексей", "Дмитрий", "Андрей", "Михаил", "Николай", "Владимир", "Александр"
};
static const char* last_names[] = {
    "Иванов", "Петров", "Сидоров", "Смирнов", "Кузнецов", "Попов", "Васильев", "Зайцев", "Соколов", "Михайлов"
};
static const char* patronymics[] = {
    "Иванович", "Петрович", "Сергеевич", "Алексеевич", "Дмитриевич", "Андреевич", "Михайлович", "Николаевич", "Владимирович", "Александрович"
};

// Список названий дисциплин
static const char* subject_names[] = {
    "Математика", "Физика", "Информатика", "История", "Философия",
    "Программирование", "Базы данных", "Компьютерные сети", "Экономика", "Английский язык"
};
static const int num_subject_names = sizeof(subject_names) / sizeof(subject_names[0]);

// Функция освобождения памяти всех студентов
void free_students() {
    if (students) {
        for (int i = 0; i < student_count; i++) {
            if (students[i].subjects)
                free(students[i].subjects);
        }
        free(students);
        students = NULL;
        student_count = 0;
    }
}

// Генерация одного предмета
void generate_subject(subject* s) {
    // название дисциплины
    int idx = rand() % num_subject_names;
    strncpy(s->name, subject_names[idx], 30);
    s->name[30] = '\0';
    // кабинеты
    s->lecture_room = (unsigned char)(rand() % 255 + 1);
    s->lab_room = (unsigned char)(rand() % 255 + 1);
    // часы (от 16 до 240)
    s->hours = (unsigned char)(rand() % 225 + 16);
    // аттестация
    s->attestation = rand() % 3;
}

// Генерация одного студента
void generate_student(student* s, int min_d, int max_d) {
    // ФИО
    snprintf(s->full_name, sizeof(s->full_name), "%s %s %s",
             last_names[rand() % (sizeof(last_names)/sizeof(last_names[0]))],
             first_names[rand() % (sizeof(first_names)/sizeof(first_names[0]))],
             patronymics[rand() % (sizeof(patronymics)/sizeof(patronymics[0]))]);
    
    // дата рождения (от 01.01.1980 до 31.12.2005)
    int year = 1980 + rand() % 26;
    int month = rand() % 12 + 1;
    int day;
    if (month == 2) {
        day = rand() % 28 + 1; // упрощённо,不考虑 високосные
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        day = rand() % 30 + 1;
    } else {
        day = rand() % 31 + 1;
    }
    snprintf(s->birth_date, sizeof(s->birth_date), "%02d.%02d.%04d", day, month, year);
    
    // группа (например, ИУ7-12Б)
    snprintf(s->group, sizeof(s->group), "ИУ7-%02d%c", rand() % 30, 'А' + rand() % 6);
    
    // количество предметов
    s->num_subjects = min_d + rand() % (max_d - min_d + 1);
    
    // выделение памяти под предметы
    s->subjects = (subject*)malloc(s->num_subjects * sizeof(subject));
    if (!s->subjects) {
        fprintf(stderr, "Ошибка выделения памяти для предметов\n");
        exit(1);
    }
    
    for (int i = 0; i < s->num_subjects; i++) {
        generate_subject(&s->subjects[i]);
    }
}

// Генерация N студентов
int generate_students(int n, int min_d, int max_d) {
    // Проверка допустимости параметров
    if (min_d < 0 || max_d < min_d) {
        fprintf(stderr, "Некорректные значения min/max дисциплин\n");
        return -1;
    }
    
    // Очищаем предыдущие данные
    free_students();
    
    // Выделяем память под массив студентов
    students = (student*)malloc(n * sizeof(student));
    if (!students) {
        fprintf(stderr, "Ошибка: недостаточно памяти для %d студентов\n", n);
        return -1;
    }
    
    student_count = n;
    
    // Генерируем каждого студента
    for (int i = 0; i < n; i++) {
        generate_student(&students[i], min_d, max_d);
    }
    
    printf("Сгенерировано %d студентов\n", n);
    return 0;
}

// Печать одного студента
void print_student(const student* s, int index) {
    printf("\n--- Студент %d ---\n", index);
    printf("ФИО: %s\n", s->full_name);
    printf("Дата рождения: %s\n", s->birth_date);
    printf("Группа: %s\n", s->group);
    printf("Предметы (%d):\n", s->num_subjects);
    for (int i = 0; i < s->num_subjects; i++) {
        printf("  %d. %s\n", i+1, s->subjects[i].name);
        printf("     Лекции: ауд.%d, лабораторные: ауд.%d, часов: %d, ",
               s->subjects[i].lecture_room, s->subjects[i].lab_room, s->subjects[i].hours);
        switch (s->subjects[i].attestation) {
            case EXAM: printf("экзамен\n"); break;
            case CREDIT: printf("зачет\n"); break;
            case COURSEWORK: printf("курсовая\n"); break;
        }
    }
}

// Вывод размера занимаемой памяти
void print_size() {
    if (student_count == 0) {
        printf("0 B\n");
        return;
    }
    
    size_t total = 0;
    for (int i = 0; i < student_count; i++) {
        total += sizeof(student); // сама структура студента
        total += students[i].num_subjects * sizeof(subject); // массив предметов
    }
    
    // Вывод в удобочитаемом формате
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit_idx = 0;
    double size = total;
    while (size >= 1024 && unit_idx < 3) {
        size /= 1024;
        unit_idx++;
    }
    printf("%.2f %s\n", size, units[unit_idx]);
}

// Очистка данных
void clean() {
    free_students();
    printf("Данные очищены\n");
}

// Разбор командной строки (интерактивный режим)
int parse_command(char* line) {
    // Удаляем символ новой строки
    line[strcspn(line, "\n")] = '\0';
    
    // Разбиваем на токены
    char* tokens[100];
    int tok_count = 0;
    char* p = strtok(line, " ");
    while (p && tok_count < 100) {
        tokens[tok_count++] = p;
        p = strtok(NULL, " ");
    }
    
    if (tok_count == 0) return 0; // пустая строка
    
    // Определяем команду
    if (strcmp(tokens[0], "exit") == 0) {
        return -1; // сигнал выхода
    }
    else if (strcmp(tokens[0], "gen") == 0) {
        if (tok_count < 2) {
            printf("Использование: gen N [--min-disciplines M] [--max-disciplines X]\n");
            return 0;
        }
        int n = atoi(tokens[1]);
        if (n <= 0) {
            printf("N должно быть положительным числом\n");
            return 0;
        }
        
        int min_d = 10;  // по умолчанию
        int max_d = 20;  // по умолчанию
        
        // Разбор опций
        for (int i = 2; i < tok_count; i++) {
            if (strcmp(tokens[i], "--min-disciplines") == 0 && i+1 < tok_count) {
                min_d = atoi(tokens[++i]);
            } else if (strcmp(tokens[i], "--max-disciplines") == 0 && i+1 < tok_count) {
                max_d = atoi(tokens[++i]);
            } else {
                printf("Неизвестная опция: %s\n", tokens[i]);
                return 0;
            }
        }
        
        generate_students(n, min_d, max_d);
    }
    else if (strcmp(tokens[0], "get_size") == 0) {
        print_size();
    }
    else if (strcmp(tokens[0], "print_students") == 0) {
        if (tok_count < 2) {
            printf("Использование: print_students N (или -1 для всех)\n");
            return 0;
        }
        int n = atoi(tokens[1]);
        if (student_count == 0) {
            printf("Нет сгенерированных данных\n");
            return 0;
        }
        if (n == -1) n = student_count;
        if (n < 0 || n > student_count) {
            printf("Некорректное значение N (доступно %d студентов)\n", student_count);
            return 0;
        }
        for (int i = 0; i < n; i++) {
            print_student(&students[i], i+1);
        }
    }
    else if (strcmp(tokens[0], "clean") == 0) {
        clean();
    }
    else {
        printf("Неизвестная команда. Доступные: gen, get_size, print_students, clean, exit\n");
    }
    return 0;
}

int main() {
    // Инициализация генератора случайных чисел
    srand(time(NULL));
    
    printf("Интерактивный генератор студентов. Введите 'exit' для выхода.\n");
    
    char line[256];
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        int rc = parse_command(line);
        if (rc == -1) break;
    }
    
    // Освобождаем память перед выходом
    free_students();
    return 0;
}