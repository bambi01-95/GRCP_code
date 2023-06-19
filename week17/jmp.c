// C program to demonstrate 
// working of setjmp() and
// longjmp()
#include <setjmp.h>
#include <stdio.h>
jmp_buf buf;

void func()
{
    printf("Welcome to GeeksforGeeks\n");
  
    // Jump to the point setup by setjmp
    longjmp(buf, 1);
    printf("doesn't reach\n");
}
  
int main()
{
    // Setup jump position using buf and return 0
    if (setjmp(buf)==2)
        printf("Geek2==1\n");
    else if(setjmp(buf)==1){
        printf("Geek2==2\n");
    }
    else {
        printf("%d--\n",setjmp(buf));
        printf("Geek1\n");
        func();
    }
    return 0;
}

/*
    memo
    setjmp(buf) return 0, if it isn't called by longjmp or not defineded
    longjmp(buf,i) that go back to buf potionsion that settted by setjmp. and then return i.
*/




/*
          this 
*/
// #include <setjmp.h>
// #include <stdio.h>
// #include <stdlib.h>

// jmp_buf jmp_div;
// jmp_buf jmp_add;
// void divide_test (int a, int b)
// {
//   if (b == 0) {
//     longjmp(jmp_div,0);
//   }
//   int div = a / b;
//   printf ("divide_test (%d, %d) = %d\n", a, b, div);
//   return;
// }


// int main()
// {
//   if (setjmp(jmp_div) == 0) {
//     divide_test (223, 7);
//     divide_test (571, 13);
//     // divide_test (311, 0);     
//   } else {
//     fprintf (stderr, "divide_test failure\n");
//     return EXIT_FAILURE;
//   }
//   if(setjmp(jmp_add)==0){
//     printf("100\n");
//     longjmp(jmp_div,1);
//   }
//   else{
//     printf("200\n");
//   }
//   printf("end of the code\n");
//   return 0;
// }




