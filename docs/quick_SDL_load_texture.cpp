SDL_Texture* LoadTexture(const char* fileName)
{
// Load from file
SDL_Surface* surf = IMG_Load(fileName);
if (!surf)
{
SDL_Log("Failed to load texture file %s", fileName);
return nullptr;
}
// Create texture from surface
SDL_Texture* text = SDL_CreateTextureFromSurface(mRenderer, surf);
SDL_FreeSurface(surf);
if (!text)
{
SDL_Log("Failed to convert surface to texture for %s", fileName);
return nullptr;
}
return text;
}
