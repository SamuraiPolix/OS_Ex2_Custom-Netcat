
# Declare non-files
.PHONY: all clean q1 q2 q3 q4 q5 q6

# Make the programs for all questions
all: q1 q2 q3 q4 q5 q6

q1:
	make -C q1 all

q2:
	make -C q2 all

q3:
	make -C q3 all

q4:
	make -C q4 all

q5:
	make -C q5 all

q6:
	make -C q6 all

# Clean programs for all questions
clean:
	make -C q1 clean
	make -C q2 clean
	make -C q3 clean
	make -C q4 clean
	make -C q5 clean
	make -C q6 clean