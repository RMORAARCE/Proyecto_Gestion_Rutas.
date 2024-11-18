#include <SFML/Graphics.hpp>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <cmath>

// Clase para representar un punto turístico
class PuntoTuristico {
public:
    std::string nombre;
    sf::Vector2f posicion;
    sf::Color color;

    PuntoTuristico(const std::string& nombre, sf::Vector2f posicion, sf::Color color)
        : nombre(nombre), posicion(posicion), color(color) {}
};

// Clase para representar una ruta turística
class RutaTuristica {
public:
    std::string nombre;
    std::list<PuntoTuristico> puntos;

    RutaTuristica(const std::string& nombre) : nombre(nombre) {}

    void agregarPunto(const PuntoTuristico& punto) {
        puntos.push_back(punto);
    }
};

// Clase principal para gestionar rutas
class GestorRutas {
private:
    sf::RenderWindow window;
    sf::Texture mapaTexture;
    sf::Sprite mapaSprite;
    std::list<RutaTuristica> rutas;
    RutaTuristica* rutaActual = nullptr;
    RutaTuristica* rutaSeleccionada = nullptr; // Ruta seleccionada para resaltado
    sf::Color colorActual = sf::Color::Red;

public:
    GestorRutas() : window(sf::VideoMode(800, 600), "Gestor de Rutas Turísticas") {
        if (!mapaTexture.loadFromFile("mapa.png")) {
            std::cerr << "Error al cargar la imagen del mapa.\n";
            exit(1);
        }
        mapaSprite.setTexture(mapaTexture);
    }

    void iniciar() {
        while (window.isOpen()) {
            procesarEventos();
            renderizar();
        }
    }

    void crearNuevaRuta(const std::string& nombre) {
        rutas.emplace_back(nombre);
        rutaActual = &rutas.back();
    }

    void agregarPuntoARutaActual(const std::string& nombre, sf::Vector2f posicion) {
        if (rutaActual) {
            rutaActual->agregarPunto(PuntoTuristico(nombre, posicion, colorActual));
        }
    }

private:
    void procesarEventos() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f posicionMouse(static_cast<float>(event.mouseButton.x),
                                               static_cast<float>(event.mouseButton.y));
                    seleccionarRuta(posicionMouse);
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::C) {
                    seleccionarColor();
                }
            }
        }
    }

    void seleccionarRuta(const sf::Vector2f& posicionMouse) {
        for (auto& ruta : rutas) {
            for (const auto& punto : ruta.puntos) {
                float distancia = std::sqrt(std::pow(posicionMouse.x - punto.posicion.x, 2) +
                                            std::pow(posicionMouse.y - punto.posicion.y, 2));
                if (distancia < 10.0f) { // Si el mouse está cerca de un punto turístico
                    rutaSeleccionada = &ruta;
                    return;
                }
            }
        }
        rutaSeleccionada = nullptr; // Ninguna ruta seleccionada
    }

    void renderizar() {
        window.clear();
        window.draw(mapaSprite);

        // Dibujar puntos de las rutas y sus curvas
        for (const auto& ruta : rutas) {
            bool esSeleccionada = (&ruta == rutaSeleccionada); // Verificar si la ruta está seleccionada
            sf::Color colorRuta = esSeleccionada ? sf::Color::Magenta : sf::Color::Yellow;

            if (ruta.puntos.size() > 1) {
                for (const auto& punto : ruta.puntos) {
                    sf::CircleShape circulo(5);
                    circulo.setPosition(punto.posicion.x - 5, punto.posicion.y - 5);
                    circulo.setFillColor(esSeleccionada ? sf::Color::Cyan : punto.color);
                    window.draw(circulo);
                }

                if (ruta.puntos.size() >= 4) {
                    dibujarBSpline(ruta.puntos, window, colorRuta);
                } else {
                    std::cerr << "La ruta \"" << ruta.nombre << "\" no tiene suficientes puntos para generar una curva B-Spline.\n";
                }
            }
        }
        window.display();
    }

    void seleccionarColor() {
        std::cout << "Seleccionar color: 1-Rojo, 2-Verde, 3-Azul: ";
        int opcion;
        std::cin >> opcion;
        switch (opcion) {
            case 1: colorActual = sf::Color::Red; break;
            case 2: colorActual = sf::Color::Green; break;
            case 3: colorActual = sf::Color::Blue; break;
            default: std::cerr << "Opción inválida, se mantiene el color actual.\n";
        }
    }

    void dibujarBSpline(const std::list<PuntoTuristico>& puntos, sf::RenderWindow& window, sf::Color colorRuta) {
        if (puntos.size() < 4) return;

        std::vector<sf::Vector2f> controlPoints;
        for (const auto& punto : puntos) {
            controlPoints.push_back(punto.posicion);
        }

        sf::VertexArray curva(sf::LineStrip);
        for (float t = 0.0f; t <= 1.0f; t += 0.01f) {
            float x = 0.0f, y = 0.0f;
            for (size_t i = 0; i < controlPoints.size(); ++i) {
                float b = baseFunction(i, 3, t);
                x += b * controlPoints[i].x;
                y += b * controlPoints[i].y;
            }
            curva.append(sf::Vertex(sf::Vector2f(x, y), colorRuta));
        }

        window.draw(curva);
    }

    float baseFunction(int i, int k, float t) {
        if (k == 1) {
            return (i <= t && t < i + 1) ? 1.0f : 0.0f;
        }
        float coef1 = (t - i) / (i + k - 1 - i);
        float coef2 = (i + k - t) / (i + k - (i + 1));
        return coef1 * baseFunction(i, k - 1, t) + coef2 * baseFunction(i + 1, k - 1, t);
    }
};

int main() {
    GestorRutas gestor;
    gestor.crearNuevaRuta("Ruta1");
    gestor.iniciar();
    return 0;
}
