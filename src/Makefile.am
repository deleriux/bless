bin_PROGRAMS = bless curse

libdir = $(PAMDIR)

bless_SOURCES = raise.c
bless_LDADD = $(CAP_LIBS)
curse_SOURCES = raise.c
curse_LDADD = $(CAP_LIBS)

install-exec-hook:
	setcap all=i $(DESTDIR)$(bindir)/bless ||: 
