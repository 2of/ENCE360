# ENCE360 2019 HTTP 1.0 Downloader


For ENCE360 2019 at the University of Canterbury New Zealand. A simple multipart HTTP 1.0 Downloader; Performs HTTP get requests for snippets of each file specified in `text_file` and merges each into combined files at `directory` using a number of threads specified by `threads_num`

***Usage:***

`./downloader <text_file> <threads_num> <directory>`



Final grade: **A: 87.5%**
Feedback is not available on this assignment at the current time.

***Notes About Submission***
There is a slight bug with downloading from hqwalls.com. It is reproducable on some machines but not all, this contributes to the overall grade. 
Also there are some formatting issues and a little unused code that contributes to the grade


***Side Note: ***
The python script runtime_csv_gen.py is a simple extra utility to write the time taken for each call to a specified CSV. You must simply duplicate the body of the code for each trial and write into the fields for each trial. Trial num, threads, and files are all specifiable. It is very rudimentary.
