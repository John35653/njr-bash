# README
This is a bash program for Linux.
## How to compile: 
Type `make` in the command line and hit enter. It will compile the program<br>
with the included Makefile and name it __njr-bash__. Otherwise just use regular<br> 
`gcc Shell.c func.c -o njr-bash`. Will use an example working directory for examples.<br> 
__Ex:__ 
```
/example/working/directory> make

or

/example/working/directory> gcc Shell.c func.c -o njr-bash
```
## How to run:
To run njr bash, type into the command line: `./njr-bash` and hit enter.<br>
```
/example/working/directory> ./njr-bash          <-- running the command
Welcome to njr-bash                             <-- will greet the user
/example/working/directory:njr-bash>            <-- now the user can enter a command(s) into the njr-bash
```
## Recommended first steps
To find built-ins for the njr-bash, type `help`. This will list all available built-ins<br>
and also directions for how to use the rest of the commands available in the<br>
 Linux terminal. The user can even use the man pages for each of the Linux commands.
 ```
/example/working/directory:njr-bash> help                                                                                                           <-- running the help command

Welcome to njr-bash, the new shell on the block!                                                                                                    <-- greets the user

    Built-in commands: cd, exit, help, pwd.

cd - Allows you to change directories and follows the same syntax as the linux built in minus the need for " " around directories with spaces.
exit - Stops and exits njr-bash.
help - Lists the njr-bash built-ins.                                                                                                                <-- showing the available built-ins
pwd - Prints the working directory that the user is currently in.
Otherwise use all of the built in linux commands, arguments, and syntax.


 ```

## Exiting the program
To exit the program at anytime, type `exit`. If you have a child process running<br> 
like `htop`, `top`, or something like `less -f` or `strace -f`, it would be easier to send<br> 
__SIGINT (Ctrl + C)__ and then type `exit`.

* Fun fact, you can even run `make` and `make clean` for the shell itself inside of the shell lol. That could turn out to be a problem, I'm sure.

## Valgrind results:
* After running all of the possible command examples from req.txt<br>

After typing `exit`, these are the results for running the shell with Valgrind:<br>

/example/working/directory:njr-bash> __exit__<br>
Thank you for using njr-bash!<br>
==7538== <br>
==7538== I refs:        367,924<br>
==7538== I1  misses:      1,615<br>
==7538== LLi misses:      1,606<br>
==7538== I1  miss rate:    0.44%<br>
==7538== LLi miss rate:    0.44%<br>
==7538== <br>
==7538== D refs:        125,457  (85,293 rd   + 40,164 wr)<br>
==7538== D1  misses:      1,997  ( 1,524 rd   +    473 wr)<br>
==7538== LLd misses:      1,632  ( 1,231 rd   +    401 wr)<br>
==7538== D1  miss rate:     1.6% (   1.8%     +    1.2%  )<br>
==7538== LLd miss rate:     1.3% (   1.4%     +    1.0%  )<br>
==7538== <br>
==7538== LL refs:         3,612  ( 3,139 rd   +    473 wr)<br>
==7538== LL misses:       3,238  ( 2,837 rd   +    401 wr)<br>
==7538== LL miss rate:      0.7% (   0.6%     +    1.0%  )<br>

When valgrind --leak-check=full ./njr-bash was run to check<br>
both the built-ins and linux commands for memory leaks, <br>
none were found. Here are the results:<br><br>
Thank you for using njr-bash!<br>
==27723== <br>
==27723== HEAP SUMMARY:<br>
==27723==     in use at exit: 0 bytes in 0 blocks<br>
==27723==   total heap usage: 2 allocs, 2 frees, 2,048 bytes allocated<br>
==27723== <br>
==27723== All heap blocks were freed -- no leaks are possible<br>
==27723== <br>
==27723== Use --track-origins=yes to see where uninitialised values come from<br>
==27723== For lists of detected and suppressed errors, rerun with: -s<br>
