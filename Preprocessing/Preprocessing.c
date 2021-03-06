# include <stdio.h>
# include <stdlib.h>
# include <err.h>
# include <assert.h>
# include <SDL/SDL.h>
# include <SDL/SDL_image.h>

/* Code optimisé pour le projet
Seule fonction utile à rappeler en cas de besoin :
preprocessing(char *name[])
Le reste c'est toutes les étapes intermédiaires.
Y'a touuutes les étapes de pre-processing = greyscale + normalized
A adapter selon nos besoins. */

static inline
Uint8* pixelref(SDL_Surface *surf, unsigned x, unsigned y)
{
  int bpp = surf->format->BytesPerPixel;
  return (Uint8*)surf->pixels + y * surf->pitch + x * bpp;
}

void wait_for_keypressed(void)
{
  SDL_Event event;
  for (;;)
  {
    SDL_PollEvent( &event );
    switch (event.type)
    {
    case SDL_KEYDOWN: return;
    default: break;
    }
  }
}

void init_sdl(void)
{
  if( SDL_Init(SDL_INIT_VIDEO)==-1 )
  {
    errx(1,"Could not initialize SDL: %s.\n", SDL_GetError());
  }
}

SDL_Surface* load_image(char *path)
{
  SDL_Surface *img;
  img = IMG_Load(path);
  if (!img)
    errx(3, "can't load %s: %s", path, IMG_GetError());
  return img;
}

Uint32 getpixel(SDL_Surface *surface, unsigned x, unsigned y)
{
  Uint8 *p = pixelref(surface, x, y);
  switch(surface->format->BytesPerPixel)
  {
  case 1:
    return *p;
  case 2:
    return *(Uint16 *)p;
  case 3:
    if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
      return p[0] << 16 | p[1] << 8 | p[2];
    else
      return p[0] | p[1] << 8 | p[2] << 16;
  case 4:
    return *(Uint32 *)p;
  }
  return 0;
}
 
void putpixel(SDL_Surface *surface, unsigned x, unsigned y, Uint32 pixel)
{
  Uint8 *p = pixelref(surface, x, y);
  switch(surface->format->BytesPerPixel)
  {
  case 1:
    *p = pixel;
    break;
  case 2:
    *(Uint16 *)p = pixel;
    break;
  case 3:
    if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
    {
      p[0] = (pixel >> 16) & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = pixel & 0xff;
    }
    else
    {
      p[0] = pixel & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = (pixel >> 16) & 0xff;
    }
    break;
  case 4:
    *(Uint32 *)p = pixel;
    break;
  }
}

void to_grey_lvl(SDL_Surface *img)
{
  uint8_t r, g, b;
  for (int i = 0; i < img->w; i++)
  {
    for (int j = 0; j < img->h; j++)
    {
      SDL_GetRGB(getpixel(img, i, j), img->format, &r, &g, &b);
      Uint32 grey = r * 0.3 + g * 0.59 + b * 0.11;
      putpixel(img, i, j, SDL_MapRGB(img->format, grey, grey, grey));
    }
  }
}

Uint32 minIntensity(SDL_Surface *img)
{
  Uint32 min = getpixel(img, 0, 0);
  for (int i = 0; i < img->w; i++)
  {
    for (int j = 0; j < img->h; j++)
    {
      Uint32 value = getpixel(img, i, j);
      if (value < min)
	min = value;
    }
  }
  return min;
}

Uint32 maxIntensity(SDL_Surface *img)
{
  Uint32 max = getpixel(img, 0, 0);
  for (int i = 0; i < img->w; i++)
  {
    for (int j = 0; j < img->h; j++)
    {
      Uint32 value = getpixel(img, i, j);
      if (value > max)
	max = value;
    }
  }
  return max;
}

void normalized(SDL_Surface *img)
{
  Uint32 min = minIntensity(img), max = maxIntensity(img);
  for (int i = 0; i < img->w; i++)
  {
    for (int j = 0; j < img->h; j++)
    {
      Uint32 norm = ((getpixel(img, i, j) - min) * 255) / (max - min);
      putpixel(img, i, j, SDL_MapRGB(img->format, norm, norm, norm));
    }
  }
} 

SDL_Surface* display_image(SDL_Surface *img)
{
  SDL_Surface *screen;
  screen = SDL_SetVideoMode(img->w, img->h, 0, SDL_SWSURFACE|SDL_ANYFORMAT);
  if ( screen == NULL )
  {
    errx(1, "Couldn't set %dx%d video mode: %s\n",
         img->w, img->h, SDL_GetError());
  }
  if (SDL_BlitSurface(img, NULL, screen, NULL) < 0)
    warnx("BlitSurface error: %s\n", SDL_GetError());
  SDL_UpdateRect(screen, 0, 0, img->w, img->h);
  wait_for_keypressed();
  return screen;
}

void preprocessing(char *name[])
{
  init_sdl();
  SDL_Surface* img = load_image(name[0]);
  display_image(img);
  to_grey_lvl(img);
  display_image(img);
  normalized(img);
  display_image(img);
  SDL_FreeSurface(img);
}

int main(int argc, char *argv[]) // The Test
{
  if (argc < 2)
    printf("There is no img, man");
  preprocessing(&argv[1]);
  return 1;
}
