CC = g++
LFLAGS = -Iinclude -lpthread
PNAME = CardGame

all :
	$(CC) Player.cpp -o Player $(LFLAGS)	
	$(CC) Controller.cpp -o Controller $(LFLAGS)
	
zip : pack	
	rm -f $(PNAME).zip
	zip -r $(PNAME) $(PNAME)
	rm -rf $(PNAME)
	
pack : unpack
	mkdir $(PNAME)
	cp *.cpp $(PNAME)
	cp README.md $(PNAME)
	cp Makefile $(PNAME)
	cp -r include $(PNAME)
	
unpack : 
	rm -rf $(PNAME)
	
clean : unpack
	rm -f Player Controller $(PNAME).zip