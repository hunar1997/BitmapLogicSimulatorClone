Get compiled binaries for the first working bare bones for windows and linux at (release) section

Re creating the bitmap logic simulator, original simulator:
https://realhet.wordpress.com/2015/09/02/bitmap-logic-simulator/
he wrote it in lua (i think) i'm rewriting it in c++ using SFML
i don't have (or don't know where to get) his source code
so i think it will be a chalanging task
here it goes

His software helped me to understand how computers work, but its been a year since i switched entierly to linux but his software only runs on windows, although it runs on [Wine], AND i wanted to write my own AND i wanted a faster one (he also wanted a faster one)


In linux you can simply [cmake .] and then [make] to get your executable, i had troubles in windows and wasted alot of time figuring out how to compile it without the (.dll)s and still not succeded enough AND its slower in windows (I've done something wrong)

This is a codelite project, you can open it in codelite and then you can right click the project there in [Custom Targets] you will find debug and release cmakes, after that you can [Build]
