This directory contains the means to perform Co-evolutionary Design Processes (SDDP)

First, the executable ./main runs a single SCDP optimization for a single input design 
and its settings inserted as input argument after the './main' run command in the terminal.
To get the available possible settings run: './main -h' as this will give an overview 
of text in the terminal of possible input arguments to configure your single SCDP run.

Second, to run numerous SCDP runs with either different settings and or designs, 
the bash shell scripts 'run_design_sequence.sh' and 'run_plot_together.sh' is made.
Both requires the input_arg.txt where each line in the .txt describes a set of input arguments.
An example of such a file is:

-i design_301 -m sd 		-n 4 -d worst -r 0.2 -o verbose plot ms_files result_line all combine_data 6
-i design_302 -m bp 		-n 4 -d worst -r 0.2 -o verbose plot ms_files result_line all combine_data 6

In this case, first the '-i design_301 -m sd 		-n 4 -d worst -r 0.2 -o verbose plot ms_files result_line all combine_data 6'
input argument are executed, whereafter the '-i design_302 -m bp 		-n 4 -d worst -r 0.2 -o verbose plot ms_files result_line all combine_data 6'
input arguments are executed.

The deference between the two is that 'run_plot_together.sh' also copy pasted the plot data output of each individual run into
a data file: 'combined_vectorData_....txt file' wherefrom a combined_plot_....png is generated utilizing GNUPlot.

By running the SCDP optimization via a bash scipt allows for multiple back to back runs without having any errors stop the back-to-back run or initiating them manually.
In case of an error in any of the runs, an error_log is exported either in the directory which it is run or in a result directory if that one is set in the input arguments for that run.

For numerous runs and comparing the output is the input argument '-o combine_data x' made (see -h for more information).
The combine_data marker generates a line of output in the .csv file which can be imported in excel for data analysis.
For every run sequence, it is advised to manually copy past the csv files to a storage directory, as a subsequential run will extend the current .csv file in case of the 'combine_data' argument
