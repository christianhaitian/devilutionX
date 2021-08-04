/**
 * @file text_render.cpp
 *
 * Text rendering.
 */
#include "text_render.hpp"

#include <array>
#include <unordered_map>
#include <utility>

#include "DiabloUI/art_draw.h"
#include "DiabloUI/ui_item.h"
#include "cel_render.hpp"
#include "engine.h"
#include "engine/load_cel.hpp"
#include "engine/point.hpp"
#include "palette.h"
#include "engine/load_file.hpp"

namespace devilution {

namespace {

std::unordered_map<int, std::reference_wrapper<Art>> UnicodeFonts;
//std::unordered_map<int, std::array<uint8_t, 256>> UnicodeFontsSpacing;
//struct RowFont {
//	std::unique_ptr<uint8_t[]> spacing[6];
//	Art MenuFonts[3][2];
//	Art GameFonts[3];
//};

std::array<const char[4], 1> ColorPrefixes = {
	//"w12",

	//"w16",
	//"g16",
	//"s16",
	//
	//"g24",
	//"s24",
	//
	"g30",
	//"s30",
	//
	//"g42",
	//"s42",
	//
	//"g46",
	//"s46",
};
Art font;
std::array<uint8_t, 256> FontKern;

void LoadArtFont(int row)
{
	for (auto ColorPrefixe : ColorPrefixes) {
		char path[32];
		sprintf(path, "fonts\\%s-%02x.pcx", ColorPrefixe, row);

		//Art *font = new Art();
		LoadMaskedArt(path, &font, 256, 1);

		sprintf(path, "fonts\\30-%02x.bin", row);
		LoadFileInMem(path, FontKern);
		for (auto &width : FontKern) {
			width -= 1;
		}
		//FontTables[AFT_SMALL] = LoadFileInMem<uint8_t>("ui_art\\font16.bin");
		//UnicodeFonts.emplace(row, *font);
	}
}

enum text_color : uint8_t {
	ColorWhite,
	ColorBlue,
	ColorRed,
	ColorGold,
	ColorBlack,
};

int LineHeights[3] = { 12, 38, 50 };

/** Graphics for the fonts */
std::array<std::optional<CelSprite>, 3> fonts;

uint8_t fontColorTableGold[256];
uint8_t fontColorTableBlue[256];
uint8_t fontColorTableRed[256];

void DrawChar(const Surface &out, Point position, GameFontTables size, int nCel, text_color color)
{
	switch (color) {
	case ColorWhite:
		CelDrawTo(out, position, *fonts[size], nCel);
		return;
	case ColorBlue:
		CelDrawLightTo(out, position, *fonts[size], nCel, fontColorTableBlue);
		break;
	case ColorRed:
		CelDrawLightTo(out, position, *fonts[size], nCel, fontColorTableRed);
		break;
	case ColorGold:
		CelDrawLightTo(out, position, *fonts[size], nCel, fontColorTableGold);
		break;
	case ColorBlack:
		LightTableIndex = 15;
		CelDrawLightTo(out, position, *fonts[size], nCel, nullptr);
		return;
	}
}

} // namespace

std::optional<CelSprite> pSPentSpn2Cels;

void InitText()
{
	LoadArtFont(0);

	fonts[GameFontSmall] = LoadCel("CtrlPan\\SmalText.CEL", 13);
	fonts[GameFontMed] = LoadCel("Data\\MedTextS.CEL", 22);
	fonts[GameFontBig] = LoadCel("Data\\BigTGold.CEL", 46);

	pSPentSpn2Cels = LoadCel("Data\\PentSpn2.CEL", 12);

	for (int i = 0; i < 256; i++) {
		uint8_t pix = i;
		if (pix >= PAL16_GRAY + 14)
			pix = PAL16_BLUE + 15;
		else if (pix >= PAL16_GRAY)
			pix -= PAL16_GRAY - (PAL16_BLUE + 2);
		fontColorTableBlue[i] = pix;
	}

	for (int i = 0; i < 256; i++) {
		uint8_t pix = i;
		if (pix >= PAL16_GRAY)
			pix -= PAL16_GRAY - PAL16_RED;
		fontColorTableRed[i] = pix;
	}

	for (int i = 0; i < 256; i++) {
		uint8_t pix = i;
		if (pix >= PAL16_GRAY + 14)
			pix = PAL16_YELLOW + 15;
		else if (pix >= PAL16_GRAY)
			pix -= PAL16_GRAY - (PAL16_YELLOW + 2);
		fontColorTableGold[i] = pix;
	}
}

int GetLineWidth(string_view text, GameFontTables size, int spacing, int *charactersInLine)
{
	int lineWidth = 0;

	size_t i = 0;
	for (; i < text.length(); i++) {
		if (text[i] == '\n')
			break;

		uint8_t frame = text[i] & 0xFF;
		lineWidth += FontKern[frame] + spacing;
	}

	if (charactersInLine != nullptr)
		*charactersInLine = i;

	return lineWidth != 0 ? (lineWidth - spacing) : 0;
}

int AdjustSpacingToFitHorizontally(int &lineWidth, int maxSpacing, int charactersInLine, int availableWidth)
{
	if (lineWidth <= availableWidth || charactersInLine < 2)
		return maxSpacing;

	const int overhang = lineWidth - availableWidth;
	const int spacingRedux = (overhang + charactersInLine - 2) / (charactersInLine - 1);
	lineWidth -= spacingRedux * (charactersInLine - 1);
	return maxSpacing - spacingRedux;
}

void WordWrapGameString(char *text, size_t width, GameFontTables size, int spacing)
{
	const size_t textLength = strlen(text);
	size_t lineStart = 0;
	size_t lineWidth = 0;
	for (unsigned i = 0; i < textLength; i++) {
		if (text[i] == '\n') { // Existing line break, scan next line
			lineStart = i + 1;
			lineWidth = 0;
			continue;
		}

		uint8_t frame = text[i] & 0xFF;
		lineWidth += FontKern[frame] + spacing;

		if (lineWidth - spacing <= width) {
			continue; // String is still within the limit, continue to the next line
		}

		size_t j; // Backtrack to the previous space
		for (j = i; j >= lineStart; j--) {
			if (text[j] == ' ') {
				break;
			}
		}

		if (j == lineStart) { // Single word longer than width
			if (i == textLength)
				break;
			j = i;
		}

		// Break line and continue to next line
		i = j;
		text[i] = '\n';
		lineStart = i + 1;
		lineWidth = 0;
	}
}

/**
 * @todo replace Rectangle with cropped Surface
 */
uint16_t DrawString(const Surface &out, string_view text, const Rectangle &rect, UiFlags flags, int spacing, int lineHeight, bool drawTextCursor)
{
	GameFontTables size = GameFontSmall;
	if (HasAnyOf(flags, UiFlags::FontMedium))
		size = GameFontMed;
	else if (HasAnyOf(flags, UiFlags::FontHuge))
		size = GameFontBig;

	text_color color = ColorGold;
	if (HasAnyOf(flags, UiFlags::ColorSilver))
		color = ColorWhite;
	else if (HasAnyOf(flags, UiFlags::ColorBlue))
		color = ColorBlue;
	else if (HasAnyOf(flags, UiFlags::ColorRed))
		color = ColorRed;
	else if (HasAnyOf(flags, UiFlags::ColorBlack))
		color = ColorBlack;

	int charactersInLine = 0;
	int lineWidth = 0;
	if (HasAnyOf(flags, (UiFlags::AlignCenter | UiFlags::AlignRight | UiFlags::KerningFitSpacing)))
		lineWidth = GetLineWidth(text, size, spacing, &charactersInLine);

	int maxSpacing = spacing;
	if (HasAnyOf(flags, UiFlags::KerningFitSpacing))
		spacing = AdjustSpacingToFitHorizontally(lineWidth, maxSpacing, charactersInLine, rect.size.width);

	Point characterPosition = rect.position;
	if (HasAnyOf(flags, UiFlags::AlignCenter))
		characterPosition.x += (rect.size.width - lineWidth) / 2;
	else if (HasAnyOf(flags, UiFlags::AlignRight))
		characterPosition.x += rect.size.width - lineWidth;

	int rightMargin = rect.position.x + rect.size.width;
	int bottomMargin = rect.size.height != 0 ? rect.position.y + rect.size.height : out.h();

	if (lineHeight == -1)
		lineHeight = LineHeights[size];

	uint16_t i = 0;
	for (; i < text.length(); i++) {
		uint8_t frame = text[i] & 0xFF;
		int symbolWidth = FontKern[frame];
		if (text[i] == '\n' || characterPosition.x + symbolWidth > rightMargin) {
			if (characterPosition.y + lineHeight >= bottomMargin)
				break;
			characterPosition.y += lineHeight;

			if (HasAnyOf(flags, (UiFlags::AlignCenter | UiFlags::AlignRight | UiFlags::KerningFitSpacing)))
				lineWidth = GetLineWidth(&text[i + 1], size, spacing, &charactersInLine);

			if (HasAnyOf(flags, UiFlags::KerningFitSpacing))
				spacing = AdjustSpacingToFitHorizontally(lineWidth, maxSpacing, charactersInLine, rect.size.width);

			characterPosition.x = rect.position.x;
			if (HasAnyOf(flags, UiFlags::AlignCenter))
				characterPosition.x += (rect.size.width - lineWidth) / 2;
			else if (HasAnyOf(flags, UiFlags::AlignRight))
				characterPosition.x += rect.size.width - lineWidth;
		}
		DrawArt(out, characterPosition.x, characterPosition.y - lineHeight, &font, frame);
		if (text[i] != '\n')
			characterPosition.x += symbolWidth + spacing;
	}
	if (drawTextCursor) {
		CelDrawTo(out, characterPosition, *pSPentSpn2Cels, PentSpn2Spin());
	}

	return i;
}

uint8_t PentSpn2Spin()
{
	return (SDL_GetTicks() / 50) % 8 + 1;
}

} // namespace devilution
