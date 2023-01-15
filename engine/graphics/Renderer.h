#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "Color.h"
#include "BlendMode.h"
#include "Viewport.h"
#include "SpriteBatch.h"
#include "RenderTexture.h"
#include <stack>
#include <cstddef>
#include <string>
#include <functional>

namespace nebula {
    namespace graphics {

        enum class RenderPass {
            Shadow,
            Opaque,
            Transparent,
            UI,
            Count
        };

        enum class BatchMode {
            Immediate,
            Batched,
            Count
        };

        struct ScopedRenderState {
            sf::View view;
            BlendState blend;
            RenderPass pass;
            bool active;

            ScopedRenderState() : view(), blend(), pass(RenderPass::Opaque), active(false) {}
        };

        struct FrameStats {
            std::size_t drawCalls;
            std::size_t vertices;
            std::size_t batches;
            std::size_t stateChanges;
            float frameTime;
            float cpuTime;

            FrameStats()
                : drawCalls(0), vertices(0), batches(0), stateChanges(0)
                , frameTime(0.f), cpuTime(0.f) {}
        };

        struct RenderPassStats {
            std::size_t drawCalls;
            std::size_t vertices;
            std::string passName;

            RenderPassStats() : drawCalls(0), vertices(0), passName("") {}
        };

        class Renderer {
        public:
            Renderer();
            explicit Renderer(sf::RenderWindow& window);
            ~Renderer();

            void setWindow(sf::RenderWindow& window);
            sf::RenderWindow* getWindow() const;

            void beginFrame();
            void endFrame();
            void present();

            void clear(const Color& color = Color::Black);
            void clear(const sf::Color& color);

            void draw(const sf::Drawable& drawable, const sf::RenderStates& states = sf::RenderStates::Default);
            void draw(const sf::VertexArray& vertexArray, const sf::RenderStates& states = sf::RenderStates::Default);
            void draw(const sf::Vertex* vertices, std::size_t count,
                      sf::PrimitiveType type, const sf::RenderStates& states = sf::RenderStates::Default);
            void draw(SpriteBatch& batch);

            void setBlendMode(const BlendState& state);
            BlendState getBlendMode() const;

            void setViewport(const sf::FloatRect& viewport);
            sf::FloatRect getViewport() const;

            void setView(const sf::View& view);
            sf::View getView() const;
            sf::View getDefaultView() const;

            void pushView(const sf::View& view);
            void popView();
            std::size_t getViewStackSize() const;

            void setRenderTarget(RenderTexture* target);
            void resetRenderTarget();

            FrameStats getFrameStats() const;
            std::size_t getDrawCalls() const;
            void resetStats();

            void setClearColor(const Color& color);
            Color getClearColor() const;

            void setVSync(bool enabled);
            bool isVSync() const;

            void setBatchMode(BatchMode mode);
            BatchMode getBatchMode() const;

            void beginRenderPass(RenderPass pass);
            void endRenderPass(RenderPass pass);
            RenderPass getCurrentRenderPass() const;

            void pushRenderState(const ScopedRenderState& state);
            void popRenderState();
            std::size_t getRenderStateStackSize() const;

            void setPassCallback(RenderPass pass, std::function<void()> callback);
            void clearPassCallbacks();

            RenderPassStats getPassStats(RenderPass pass) const;
            std::size_t getStateChanges() const;
            void setBatchFlushCallback(std::function<void()> callback);

            bool isBatchRendering() const;

        private:
            sf::RenderWindow* m_window;
            RenderTexture* m_customTarget;
            sf::RenderTarget* m_activeTarget;

            FrameStats m_stats;
            RenderPassStats m_passStats[static_cast<int>(RenderPass::Count)];
            bool m_frameBegan;
            Color m_clearColor;
            BlendState m_blendState;
            sf::FloatRect m_viewport;

            BatchMode m_batchMode;
            RenderPass m_currentPass;
            std::stack<ScopedRenderState> m_stateStack;
            std::function<void()> m_passCallbacks[static_cast<int>(RenderPass::Count)];
            std::function<void()> m_batchFlushCallback;

            std::stack<sf::View> m_viewStack;
            sf::View m_defaultView;

            void applyState();
        };

    }
}
