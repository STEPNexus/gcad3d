#

$(info - entering src2obj.mak)

OBJ1 = $(patsubst %.c,%.o, $(notdir $(SRC1)))
OBJG = $(patsubst %.c,%.o, $(notdir $(SRCG)))
OBJA = $(patsubst %.c,%.o, $(notdir $(SRCA)))
OBJOS = $(patsubst %.c,%.o, $(notdir $(SRCOS)))
OBJGUI = $(patsubst %.c,%.o, $(notdir $(SRCGUI)))
OBJGLB = $(patsubst %.c,%.o, $(notdir $(SRCGLB)))


VPATH=$(gcad_dir_bin):../xa/:../exp/:../db/:../ci/:../ut/:../gr/:../gl2ps/:../gui/:../gui_$(VGUI)/

# MSYS: VPATH does not work with blank in pathname .. ??


# EOF
