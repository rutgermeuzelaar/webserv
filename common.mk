CC := c++
INC_DIRS := include
INC := $(foreach dir, $(INC_DIRS), -I$(dir))
CFLAGS := -Wall -Wextra -Werror -Wshadow $(INC) -g -std=c++11
LDFLAGS :=
SRCDIR := src
OBJDIR := build
OBJECTS_STANDARD := $(SOURCES_STANDARD:%.cpp=$(OBJDIR)/%.o)
OBJECTS_SHARED := $(SOURCES_SHARED:%.cpp=$(OBJDIR)/%.o)
OBJECTS_BONUS := $(SOURCES_BONUS:%.cpp=$(OBJDIR)/%.o)

.PHONY: all

all: $(NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(NAME): $(OBJECTS_STANDARD) $(OBJECTS_SHARED)
	$(CC) $(OBJECTS_STANDARD) $(OBJECTS_SHARED) -o $(NAME)

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm -rf $(OBJECTS_STANDARD) $(OBJECTS_SHARED) $(OBJECTS_BONUS)

fclean: clean
	rm -rf $(NAME)

re: fclean all
