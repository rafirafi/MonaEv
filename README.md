# MonaEv : 
GpGpu polygon painter  

Inspired by http://alteredqualia.com/visualization/evolve 


Requires :  
Opengl >= 2.1 + some extensions (fbo, half float texture) or Opengl >= 3.0  
Platform : linux (ok), windows (seems ok), darwin (not tested)  

P.ex with an old laptop with intel GMA4500M and 1000 triangles : 

-with 1 slice : 110 changes / seconde
```
$sudo intel_gpu_time ./monaev --image mona.png --slicenb 1 --polyvertexcount 1000,3  
          score   mutation    improv.    neutral child. fitness change/s  
...  
        7823338       1400         56          0     25 75.047  108.52  
        7805378       1525         61          0     25 75.105  108.73  
user: 1.056000s, sys: 0.328000s, elapsed: 14.903300s, CPU: 9.3%, GPU: 97.8%  
```
-with 8 slices : 190 changes / seconde
```
$sudo  intel_gpu_time ./monaev --image mona.png --slicenb 8 --polyvertexcount 1000,3  
          score   mutation    improv.    neutral child. fitness change/s  
...  
        6499518       2600        104          0     25 79.270  188.48  
user: 2.672000s, sys: 0.636000s, elapsed: 14.695536s, CPU: 22.5%, GPU: 97.1%  
```

I didn't focus on how the change/mutation are done so a local optimum is relatively quickly reached.  
I always use the same image when testing, so with a good mutation selection this quality could be reached.


```
USAGE: 	monaev --image path --dna path [OPTIONS]
	    monaev --image path --polyvertexcount polygon_nb,vertex_nb [OPTIONS]
Then with the focus on the window, Interactive Command Keys:
            Esc/Enter   : quit
            Space       : print dna
            T           : switch displayed image/shapes
            P           : pause/unpause
            S           : print svg
            H           : display Interactive Command Keys help
Mouse Command:
			Left double click :	Resize window to image size.

OPTIONS:

--descendantnb ARG              Nb of descendant per generation.
--dna ARG                       Path to a dna file associated with the image to
                                paint.
--dropneutral                   Drop neutral changes.
--force16bit                    Always uses 16 bit textures for evaluation
                                calculations. Always set for opengl 2.1.
                                Variable decrease of the performance, increase
                                of the gpu load.
--help                          Display this help.
--image ARG                     Path to the png image to paint. [mandatory]
--polyvertexcount ARG1[,ARGn]   Number of polygon (>= 1) followed by number of
                                vertex (>= 3) to paint, separated by a comma
--slicenb ARG                   Divide polygon rendering for reuse [1,polygon
                                count], if 0 calibrate for performance, if not
                                set use 1 slice. Evaluation results slightly
                                change with the nb of slice.
--ssd                           Evaluate with sum of squared differences instead
                                of sum of absolute differences.
EXAMPLES:

	monaev --image mona.png --dna mona.txt --descendantnb 100
	monaev --image bear.png --polyvertexcount 1000,4 --ssd --dropneutral
```
