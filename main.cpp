#include <iostream>
#include <cstring>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct Options {
    const char* inputImage{ nullptr };
    const char* outputJsonPalette{ nullptr };
    const char* outputHtmlPalette{ nullptr };
};

struct Base16Palette {
    unsigned int primary[8][3]{};
    unsigned int accents[8][3]{};
};

float GetMinChannelValue(float r, float g, float b) {
    r = r < g ? r : g;
    return r < b ? r : b;
}

float GetMaxChannelValue(float r, float g, float b) {
    r = r > g ? r : g;
    return r > b ? r : b;
}

float GetColorBrightness(float r, float g, float b) {
    return std::clamp((r * 0.299f) + (g * 0.587f) + (b * 0.114f), 0.0f, 1.0f);
}

float GetColorSaturation(float r, float g, float b) {
    float min = GetMinChannelValue(r, g, b) / 255.0;
    float max = GetMaxChannelValue(r, g, b) / 255.0;

    if (!max) //Color is black so saturation is 0
        return 0;

    float l = max - min;

    if (l <= 0.5)
        return l / (max + min);
    return std::clamp(l / (2 - max - min), 0.0f, 1.0f);
}

float GetColorHUE(float r, float g, float b) {
    r /= 255.0;
    g /= 255.0;
    b /= 255.0;
    
    float min = GetMinChannelValue(r, g, b);
    float max = GetMaxChannelValue(r, g, b);

    if (max == 0 || max == min)
        return 0;
    
    float v = 0;

    if (b == max)
        v = 4.0 + (r - g) / (max - min);
    if (g == max)
        v = 2.0 + (b - r) / (max - min);
    if (r == max)
        v = (g - b) / (max - min);
    
    v *= 60;
    v = v < 0 ? v + 360 : v;

    return std::clamp(v, 0.0f, 360.0f);
}

float f(float n, float h, float s, float l) {
    float a = s * std::min(l, 1.0f - l);
    float k = fmod((n + h / 30), 12);
    return l - a * std::max(std::min(std::min(k - 3.0f, 9.0f - k), 1.0f), -1.0f);
}

void Hsl2Rgb(float h, float s, float l, float* r, float* g, float* b) {
    *r = f(0, h, s, l) * 255;
    *g = f(8, h, s, l) * 255;
    *b = f(4, h, s, l) * 255;
}

void WriteJsonPalette(const Base16Palette& palette, const char* path) {
    
}

void WriteHtmlPalette(const Base16Palette& palette, const char* path) {
    char buff[65536]{};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(buff, 65536, R"HTML(
    <html>
        <head>
            <style>
                body {
                    margin: 2rem;
                    font-family: sans-serif;
                }
                body > div {
                    display: flex;
                    flex-direction: row;
                    flex-wrap: wrap;
                    gap: 1em;
                }
                body > div > div {
                    width: 5rem;
                    height: 5rem;
                    display: flex;
                    justify-content: center;
                    align-items: center;
                }
                #base00 { background-color: #%02x%02x%02x; color: #cdd6f4; }
                #base01 { background-color: #%02x%02x%02x; color: #cdd6f4; }
                #base02 { background-color: #%02x%02x%02x; color: #cdd6f4; }
                #base03 { background-color: #%02x%02x%02x; color: #1e1e2e; }
                #base04 { background-color: #%02x%02x%02x; color: #1e1e2e; }
                #base05 { background-color: #%02x%02x%02x; color: #1e1e2e; }
                #base06 { background-color: #%02x%02x%02x; color: #1e1e2e; }
                #base07 { background-color: #%02x%02x%02x; color: #1e1e2e; }
                #base08 { background-color: #%02x%02x%02x; color: #1e1e2e; }
                #base09 { background-color: #%02x%02x%02x; color: #1e1e2e; }
                #base0A { background-color: #%02x%02x%02x; color: #1e1e2e; }
                #base0B { background-color: #%02x%02x%02x; color: #1e1e2e; }
                #base0C { background-color: #%02x%02x%02x; color: #1e1e2e; }
                #base0D { background-color: #%02x%02x%02x; color: #1e1e2e; }
                #base0E { background-color: #%02x%02x%02x; color: #1e1e2e; }
                #base0F { background-color: #%02x%02x%02x; color: #1e1e2e; }
            </style>
        </head>
        <body>
            <h2>Primary colors</h2>
            <div>
                <div id="base00">00</div>
                <div id="base01">01</div>
                <div id="base02">02</div>
                <div id="base03">03</div>
                <div id="base04">04</div>
                <div id="base05">05</div>
                <div id="base06">06</div>
                <div id="base07">07</div>
            </div>
            <h2>Accents</h2>
            <div>
                <div id="base08">08</div>
                <div id="base09">09</div>
                <div id="base0A">0A</div>
                <div id="base0B">0B</div>
                <div id="base0C">0C</div>
                <div id="base0D">0D</div>
                <div id="base0E">0E</div>
                <div id="base0F">0F</div>
            </div>
        </body>
    </html>
    )HTML", 
    palette.primary[0][0], palette.primary[0][1], palette.primary[0][2], 
    palette.primary[1][0], palette.primary[1][1], palette.primary[1][2], 
    palette.primary[2][0], palette.primary[2][1], palette.primary[2][2], 
    palette.primary[3][0], palette.primary[3][1], palette.primary[3][2], 
    palette.primary[4][0], palette.primary[4][1], palette.primary[4][2], 
    palette.primary[5][0], palette.primary[5][1], palette.primary[5][2], 
    palette.primary[6][0], palette.primary[6][1], palette.primary[6][2], 
    palette.primary[7][0], palette.primary[7][1], palette.primary[7][2], 

    palette.accents[0][0], palette.accents[0][1], palette.accents[0][2], 
    palette.accents[1][0], palette.accents[1][1], palette.accents[1][2], 
    palette.accents[2][0], palette.accents[2][1], palette.accents[2][2], 
    palette.accents[3][0], palette.accents[3][1], palette.accents[3][2], 
    palette.accents[4][0], palette.accents[4][1], palette.accents[4][2], 
    palette.accents[5][0], palette.accents[5][1], palette.accents[5][2], 
    palette.accents[6][0], palette.accents[6][1], palette.accents[6][2], 
    palette.accents[7][0], palette.accents[7][1], palette.accents[7][2]);
#pragma GCC diagnostic pop

    std::ofstream file{};
    file.open(path);
    file << buff;
    file.close();
}

#define HUE_VALUE_COUNT 360
#define SATURATION_VALUE_COUNT 100
#define BRIGHTNESS_VALUE_COUNT 100

struct Base16HSLPalette {
    unsigned int primary[8][3]{};
    unsigned int accents[8][3]{};
};

struct KeySL {
    int population{};
    int brightness{};
    int averageSaturation{};
};

struct KeyHUE {
    int population{};
    int hue{};
};

Base16Palette PaletteHSLtoRGB(const Base16HSLPalette& palette) {
    Base16Palette paletteRGB{};
    
    for (int i = 0; i < 8; ++i) {
        float r, g, b;
        float hue = (float)palette.accents[i][0];
        float saturation = (float)palette.accents[i][1] / (float)SATURATION_VALUE_COUNT;
        float brightness = (float)palette.accents[i][2] / (float)BRIGHTNESS_VALUE_COUNT;

        Hsl2Rgb(hue, saturation, brightness, &r, &g, &b);
        paletteRGB.accents[i][0] = r;
        paletteRGB.accents[i][1] = g;
        paletteRGB.accents[i][2] = b;
    }

    return paletteRGB;
}

void ExtractPaletteFromImage(unsigned char* data, int width, int height, int channels, Base16Palette& palette) {
    int populationBrightness[BRIGHTNESS_VALUE_COUNT]{};
    int populationHUE[HUE_VALUE_COUNT]{};
    int populationPerBrightnessSaturation[BRIGHTNESS_VALUE_COUNT][SATURATION_VALUE_COUNT]{};
    int totalPopulation = width * height;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = x + y * width;
            float r = data[idx * channels] / 255.0;
            float g = data[idx * channels + 1] / 255.0;
            float b = data[idx * channels + 2] / 255.0;
            int hueIdx = (int)GetColorHUE(r, g, b);
            int saturationIdx = (int)(GetColorSaturation(r, g, b) * SATURATION_VALUE_COUNT);
            int brightnessIdx = (int)(GetColorBrightness(r, g, b) * BRIGHTNESS_VALUE_COUNT);
            populationBrightness[brightnessIdx] += 1;
            populationHUE[hueIdx] += 1;
            populationPerBrightnessSaturation[brightnessIdx][saturationIdx] += 1;
        }
    }

    int averageSaturationPerBrightness[BRIGHTNESS_VALUE_COUNT]{};

    for (int i = 0; i < BRIGHTNESS_VALUE_COUNT; ++i) {
        int accSat = 0;
        int c = 0;
        for (int j = 0; j < SATURATION_VALUE_COUNT; ++j) {
            accSat += j * populationPerBrightnessSaturation[i][j];
            c += populationPerBrightnessSaturation[i][j];
        }
        if (c == 0)
            averageSaturationPerBrightness[i] = 0;
        else
            averageSaturationPerBrightness[i] = accSat / c;
    }

    std::vector<KeySL> keySL{};

    for (int i = 0; i < BRIGHTNESS_VALUE_COUNT; ++i) {
        int beforePop = 0;
        int currentPop = populationBrightness[i];
        int afterPop = 0;
        
        if (i > 0)
            beforePop = populationBrightness[i - 1];
        if (i < BRIGHTNESS_VALUE_COUNT - 1)
            afterPop = populationBrightness[i + 1];

        if (currentPop > beforePop && currentPop > afterPop && currentPop > 1000) {
            KeySL info{};
            info.population = currentPop;// + beforePop + afterPop;
            info.brightness = i;
            info.averageSaturation = averageSaturationPerBrightness[i];
            keySL.push_back(info);
        }
    }

    std::vector<KeyHUE> popHUE{};

    for (int i = 0; i < HUE_VALUE_COUNT; ++i) {
        KeyHUE ph{};
        ph.hue = i;
        ph.population = populationHUE[i];
        if (ph.population > totalPopulation / 200)
            popHUE.push_back(ph);
    }

    std::sort(popHUE.begin(), popHUE.end(), [](const KeyHUE& a, const KeyHUE& b){
        return a.hue < b.hue;
    });

    int minDiff = 0;
    int diffThreshold = 15;

    while(minDiff < diffThreshold) {
        minDiff = diffThreshold;

        std::sort(popHUE.begin(), popHUE.end(), [](const KeyHUE& a, const KeyHUE& b){
            return a.hue < b.hue;
        });

        for (int i = 0; i < popHUE.size(); ++i) {
            KeyHUE keyA = popHUE[i];
            KeyHUE keyB = popHUE[(i + 1) % popHUE.size()];

            int rawDiff = abs(keyA.hue - keyB.hue);
            int currDiff = rawDiff;
            if (rawDiff > 45)
                int currDiff = sin((float)rawDiff / (float)HUE_VALUE_COUNT * M_PI / 180.0f) * 360;

            if (currDiff < diffThreshold) {
                minDiff = currDiff;

                int hueAverage = 0;
                if (rawDiff <= 180) {
                    hueAverage = keyA.hue * keyA.population + keyB.hue * keyB.population;
                } else {
                    hueAverage = ((keyA.hue + 360) * keyA.population + keyB.hue * keyB.population) % 360;
                }
                
                KeyHUE ph{};
                ph.population = keyA.population + keyB.population;
                if (hueAverage != 0)
                    ph.hue = hueAverage / ph.population;
                popHUE.erase(popHUE.begin() + i);
                popHUE.erase(popHUE.begin() + (i % popHUE.size()));
                popHUE.push_back(ph);
                break;
            }
        }
    }

    for (int i = 0; i < popHUE.size();) {
        if (popHUE[i].population > totalPopulation / 20)
            ++i;
        else
            popHUE.erase(popHUE.begin() + i);
    }

    std::sort(popHUE.begin(), popHUE.end(), [](const KeyHUE& a, const KeyHUE& b){
        return a.population > b.population;
    });

    for (int i = 0; i < popHUE.size(); ++i) {
        KeyHUE ph = popHUE[i];
        printf("Key HUE: (population: %d, hue: %d)\n", ph.population, ph.hue);
    }

    std::sort(keySL.begin(), keySL.end(), [](const KeySL& a, const KeySL& b){
        return a.population > b.population;
    });

    for (int i = 0; i < keySL.size(); ++i) {
        KeySL info = keySL[i];
        printf("Key SL: (population: %d, brightness: %d, average saturation: %d)\n", info.population, info.brightness, info.averageSaturation);
    }

    //2 score, a matching score, simply the accumulated population of each color in the palette related to the img
    //And a prettyness score, wich try to evaluate how coherent and pretty a palette is
}

void GetOptions(int argc, char* argv[], Options& options) {
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-i") == 0) {
            ++i;
            if (i >= argc)
                break;
            options.inputImage = argv[i];
        }
        if (strcmp(argv[i], "--json") == 0) {
            ++i;
            if (i >= argc)
                break;
            options.outputJsonPalette = argv[i];
        }
        if (strcmp(argv[i], "--html") == 0) {
            ++i;
            if (i >= argc)
                break;
            options.outputHtmlPalette = argv[i];
        }
        ++i;
    }
}

int main(int argc, char* argv[]) {
    Options options{};
    GetOptions(argc, argv, options);

    if (!options.inputImage) {
        std::cerr << "Input image is missing. use -i <image>." << std::endl;
        return -1;
    }

    int width, height, channels;
    unsigned char* data = stbi_load(options.inputImage, &width, &height, &channels, 3);

    if (!data) {
        std::cerr << "Couldn't load the image." << std::endl;
        return -1;
    }

    std::cout << "Porcessing image \"" << options.inputImage << "\"..." << std::endl;

    Base16Palette palette{};
    ExtractPaletteFromImage(data, width, height, channels, palette);

    if (options.outputHtmlPalette) {
        WriteHtmlPalette(palette, options.outputHtmlPalette);
        std::cout << "Writing HTML palette to \"" << options.outputHtmlPalette << "\"." << std::endl;
    }

    return 0;
}