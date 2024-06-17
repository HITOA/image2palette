#include <iostream>
#include <cstring>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define HUE_VALUE_COUNT 360
#define SATURATION_VALUE_COUNT 100
#define BRIGHTNESS_VALUE_COUNT 100

struct Options {
    const char* inputImage{ nullptr };
    const char* outputJsonPalette{ nullptr };
    const char* outputHtmlPalette{ nullptr };
};

struct ColorHSL {
    unsigned int hue{};
    unsigned int saturation{};
    unsigned int brightness{};
};

struct Color {
    unsigned char r{};
    unsigned char g{};
    unsigned char b{};
};

template<typename T>
struct Scored {
    int score{};
    T data{};
};

struct Base16HSLPalette {
    ColorHSL primary[8]{};
    ColorHSL accents[8]{};
};

struct Base16Palette {
    Color primary[8]{};
    Color accents[8]{};
};

struct KeyHS {
    int population{};
    float hue{};
    float saturation{};
};

struct KeyL {
    int population{};
    float brightness{};
    std::vector<KeyHS> keyHSs{};
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

float HueToRgb(float p, float q, float t) {
    printf("%f, %f, %f\n", p, q, t);
    if (t < 0.0f) t += 1;
    if (t > 1.0f) t -= 1;
    if (t < 1.0f/6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f/2.0f) return q;
    if (t < 2.0f/3.0f) return p + (q - p) * (2.0f/3.0f - t) * 6.0f;
    return p;
}

Color Hsl2Rgb(const ColorHSL& hslColor) {
    float h = (float)hslColor.hue / (float)HUE_VALUE_COUNT;
    float s = (float)hslColor.saturation / (float)SATURATION_VALUE_COUNT;
    float l = (float)hslColor.brightness / (float)BRIGHTNESS_VALUE_COUNT;

    Color color{};
    if (s == 0) {
        color.r = l;
        color.g = l;
        color.b = l;
    } else {
        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;
        color.r = HueToRgb(p, q, h + 1.0f / 3.0f) * 255;
        color.g = HueToRgb(p, q, h) * 255;
        color.b = HueToRgb(p, q, h - 1.0f / 3.0f) * 255;
    }
    
    return color;
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
    palette.primary[0].r, palette.primary[0].g, palette.primary[0].b, 
    palette.primary[1].r, palette.primary[1].g, palette.primary[1].b, 
    palette.primary[2].r, palette.primary[2].g, palette.primary[2].b, 
    palette.primary[3].r, palette.primary[3].g, palette.primary[3].b, 
    palette.primary[4].r, palette.primary[4].g, palette.primary[4].b, 
    palette.primary[5].r, palette.primary[5].g, palette.primary[5].b, 
    palette.primary[6].r, palette.primary[6].g, palette.primary[6].b, 
    palette.primary[7].r, palette.primary[7].g, palette.primary[7].b, 

    palette.accents[0].r, palette.accents[0].g, palette.accents[0].b, 
    palette.accents[1].r, palette.accents[1].g, palette.accents[1].b, 
    palette.accents[2].r, palette.accents[2].g, palette.accents[2].b, 
    palette.accents[3].r, palette.accents[3].g, palette.accents[3].b, 
    palette.accents[4].r, palette.accents[4].g, palette.accents[4].b, 
    palette.accents[5].r, palette.accents[5].g, palette.accents[5].b, 
    palette.accents[6].r, palette.accents[6].g, palette.accents[6].b, 
    palette.accents[7].r, palette.accents[7].g, palette.accents[7].b);
#pragma GCC diagnostic pop

    std::ofstream file{};
    file.open(path);
    file << buff;
    file.close();
}

Base16Palette PaletteHSLtoRGB(const Base16HSLPalette& palette) {
    Base16Palette paletteRGB{};
    
    for (int i = 0; i < 8; ++i) {
        paletteRGB.primary[i] = Hsl2Rgb(palette.primary[i]);
        paletteRGB.accents[i] = Hsl2Rgb(palette.accents[i]);
    }

    return paletteRGB;
}

int CalculateDifferenceMatchingScore(int a, int b, int target) {
    return abs(abs(a - b) - target);
}

float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void ExtractPaletteFromImage(unsigned char* data, int width, int height, int channels, Base16Palette& palette) {
    int totalPopulation = width * height;
    int populationBrightness[BRIGHTNESS_VALUE_COUNT]{};

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = x + y * width;
            float r = data[idx * channels] / 255.0;
            float g = data[idx * channels + 1] / 255.0;
            float b = data[idx * channels + 2] / 255.0;
            //int hueIdx = (int)GetColorHUE(r, g, b);
            //int saturationIdx = (int)(GetColorSaturation(r, g, b) * SATURATION_VALUE_COUNT);
            int brightnessIdx = (int)(GetColorBrightness(r, g, b) * BRIGHTNESS_VALUE_COUNT);
            populationBrightness[brightnessIdx] += 1;
        }
    }

    std::vector<KeyL> keyLs{};

    for (int i = 0; i < BRIGHTNESS_VALUE_COUNT; ++i) {
        if (populationBrightness[i] > totalPopulation / 1000) {
            KeyL keyL{};
            keyL.population = populationBrightness[i];
            keyL.brightness = i;
            keyLs.push_back(keyL);
        }
    }

    for (int i = 0; i < keyLs.size() - 1;) {
        KeyL keyA = keyLs[i];
        KeyL keyB = keyLs[i + 1];

        if (keyB.brightness - keyA.brightness < 8.0f) {
            KeyL newKey{};
            newKey.population = keyA.population + keyB.population;
            newKey.brightness = (keyA.brightness * keyA.population + keyB.brightness * keyB.population) / newKey.population;
            keyLs.erase(keyLs.begin() + i);
            keyLs.erase(keyLs.begin() + i);
            keyLs.insert(keyLs.begin() + i, newKey);
        } else {
            ++i;
        }
    }

    std::vector<int[HUE_VALUE_COUNT][SATURATION_VALUE_COUNT + 1]> keyLsPopulationsHUE( keyLs.size() );
    {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = x + y * width;
                float r = data[idx * channels] / 255.0;
                float g = data[idx * channels + 1] / 255.0;
                float b = data[idx * channels + 2] / 255.0;
                int hueIdx = (int)GetColorHUE(r, g, b);
                int saturationIdx = (int)(GetColorSaturation(r, g, b) * SATURATION_VALUE_COUNT);
                int brightnessIdx = (int)(GetColorBrightness(r, g, b) * BRIGHTNESS_VALUE_COUNT);
                int lastDiff{ BRIGHTNESS_VALUE_COUNT };
                for (int i = 0; i < keyLs.size(); ++i) {
                    int currentDiff = abs(brightnessIdx - keyLs[i].brightness);
                    if (currentDiff > lastDiff) {
                        keyLsPopulationsHUE[i - 1][hueIdx][0] += 1;
                        keyLsPopulationsHUE[i - 1][hueIdx][saturationIdx + 1] += 1;
                        break;
                    } else if (i == keyLs.size() - 1) {
                        keyLsPopulationsHUE[i][hueIdx][0] += 1;
                        keyLsPopulationsHUE[i][hueIdx][saturationIdx + 1] += 1;
                        break;
                    }
                    lastDiff = currentDiff;
                }
            }
        }
    }

    for (int i = 0; i < keyLsPopulationsHUE.size(); ++i) {
        for (int j = 0; j < HUE_VALUE_COUNT; ++j) {
            if (keyLsPopulationsHUE[i][j][0] > keyLs[i].population / 500) {
                KeyHS keyHS{};
                keyHS.population = keyLsPopulationsHUE[i][j][0];
                keyHS.hue = j;
                for (int k = 0; k < SATURATION_VALUE_COUNT; ++k)
                    keyHS.saturation += k * keyLsPopulationsHUE[i][j][k];
                keyHS.saturation /= keyHS.population;
                keyLs[i].keyHSs.push_back(keyHS);
            }
        }

        std::vector<KeyHS>& keyHSs = keyLs[i].keyHSs;
        for (int j = 0; j < keyHSs.size() && keyHSs.size() >= 2;) {
            KeyHS keyA = keyHSs[j];
            KeyHS keyB = keyHSs[(j + 1) % keyHSs.size()];

            float rawDiff = abs(keyA.hue - keyB.hue);
            float currentDiff = abs(abs(fmod(rawDiff * 2 / 360, 2) - 1) - 1) * 180;

            if (currentDiff < 20.0f) {
                KeyHS newKey{};
                newKey.population = keyA.population + keyB.population;
                newKey.saturation = (keyA.saturation * keyA.population + keyB.saturation * keyB.population) / newKey.population;
                if (rawDiff <= 180) {
                    newKey.hue = (keyA.hue * keyA.population + keyB.hue * keyB.population) / newKey.population;
                    keyHSs.erase(keyHSs.begin() + j);
                    keyHSs.erase(keyHSs.begin() + j % keyHSs.size());
                    keyHSs.insert(keyHSs.begin() + j, newKey);
                } else {
                    newKey.hue = fmod((keyA.hue + 360.0) * keyA.population + keyB.hue * keyB.population, 360) / newKey.population;
                    keyHSs.erase(keyHSs.begin() + j);
                    keyHSs.erase(keyHSs.begin() + j % keyHSs.size());
                    for (int k = 0; k < keyHSs.size(); ++k) {
                        if (keyHSs[k].hue > newKey.hue) {
                            keyHSs.insert(keyHSs.begin() + k, newKey);
                            break;
                        }
                    }
                }
            } else {
                ++j;
            }
        }

        std::sort(keyHSs.begin(), keyHSs.end(), [](const KeyHS& a, const KeyHS& b){
            return a.population > b.population;
        });
    }

    std::sort(keyLs.begin(), keyLs.end(), [](const KeyL& a, const KeyL& b){
        return a.population > b.population;
    });

    Base16HSLPalette hslPalette{};

    ColorHSL basePrimaryColor{};
    basePrimaryColor.hue = keyLs[0].keyHSs[0].hue;
    basePrimaryColor.saturation = keyLs[0].keyHSs[0].saturation;
    basePrimaryColor.brightness = keyLs[0].brightness;

    basePrimaryColor.brightness = std::clamp(basePrimaryColor.brightness, 10U, 90U);

    std::vector<Scored<ColorHSL>> secondPrimaryColorScore{};

    const int targetDifferenceHUE = 50;
    const int targetDifferenceSaturation = 0;
    const int targetDifferenceBrightness = 80;
    for (auto keyL : keyLs) {
        int brightnessMatchingScore = CalculateDifferenceMatchingScore(keyL.brightness, basePrimaryColor.brightness, targetDifferenceBrightness);
        for (auto keyHS : keyL.keyHSs) {
            int popularityScore = 100.0f - ((float)keyHS.population / (float)keyL.population * 100.0f);
            int hueMatchingScore = CalculateDifferenceMatchingScore(keyHS.hue, basePrimaryColor.hue, targetDifferenceHUE);
            int saturationMatchingScore = CalculateDifferenceMatchingScore(keyHS.saturation, basePrimaryColor.saturation, targetDifferenceSaturation);
            
            Scored<ColorHSL> scoredColor{};
            scoredColor.score = brightnessMatchingScore + hueMatchingScore * 0.5 + saturationMatchingScore * 0.25 + popularityScore * 0.5;
            scoredColor.data.hue = keyHS.hue;
            scoredColor.data.saturation = keyHS.saturation;
            scoredColor.data.brightness = keyL.brightness;
            secondPrimaryColorScore.push_back(scoredColor);
        }
    }

    std::sort(secondPrimaryColorScore.begin(), secondPrimaryColorScore.end(), [](const Scored<ColorHSL>& a, const Scored<ColorHSL>& b){
        return a.score < b.score;
    });

    for (int i = 0; i < 6; ++i) {
        float t = (float)i / 6.0f;
        hslPalette.primary[i].hue = Lerp(basePrimaryColor.hue, secondPrimaryColorScore[0].data.hue, t);
        hslPalette.primary[i].saturation = Lerp(basePrimaryColor.saturation, secondPrimaryColorScore[0].data.saturation, t);
        hslPalette.primary[i].brightness = Lerp(basePrimaryColor.brightness, secondPrimaryColorScore[0].data.brightness, t);
    }

    std::swap(hslPalette.primary[0], hslPalette.primary[1]);

    palette = PaletteHSLtoRGB(hslPalette);

    for (auto keyL : keyLs) {
        printf("KeyL: { population: %d%%, brightness: %d%% }\n", (int)((float)keyL.population / (float)totalPopulation * 100.0), (int)keyL.brightness);
        for (auto keyHS : keyL.keyHSs) {
            printf("\t- KeyHS: { population: %d%%, hue: %d, saturation: %d%%}\n", (int)((float)keyHS.population / (float)keyL.population * 100.0), (int)keyHS.hue, (int)keyHS.saturation);
        }
    }
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