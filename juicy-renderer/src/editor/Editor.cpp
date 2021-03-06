#include "Editor.h"

#include "EditorUtil.h"
#include "components/Components.h"
#include "editor/DiffUtil.h"
#include "framework/Framework.h"
#include "framework/Window.h"

#include <iostream>

namespace JR {

using namespace Components;

Editor::Editor(ECS& ecs)
    : mECS(ecs)
    , mInspector(ecs)
    , mHierarchy(ecs) {
	mKeyPressToken = MM::Get<Window>().Subscribe<EventKeyPress>([&](const auto& e) {
		if (!mProjectManager.IsLoaded()) {
			return;
		}
		HandleKeyPress(e);
	});

	mTransactionToken = MM::Get<TransactionManager>().Subscribe<EventTransaction>([&](const EventTransaction& message) {
		auto it = mTransactionHandlers.find(message.componentId);
		if (it == mTransactionHandlers.end()) {
			LOG_ERROR("Failed to find transaction handler for component with id: %d!", message.componentId);
			return;
		}

		it->second(message);
	});
}

void Editor::Update() {
	if (!mShowEditor) {
		return;
	}

	DrawDockSpace();

	if (!mProjectManager.IsLoaded()) {
		mProjectManager.SelectProject();
		return;
	}

	DrawMenuBar();

	static bool demo = true;
	if (demo) {
		ImGui::ShowDemoWindow(&demo);
	}

	mInspector.Update();
	mHistory.Update();
	mHierarchy.Update();
	mContentBrowser.Update();
	mViewport.Update();

	DiffUtil::FlushChanges();
}

void Editor::DrawDockSpace() {
	ImGuiWindowFlags dockSpaceWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
	                                        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
	                                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
	                                        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
	ImGui::Begin("DOCK_SPACE", nullptr, dockSpaceWindowFlags);

	ImGui::DockSpace(ImGui::GetID("DOCK_SPACE_WINDOW"), {0.f, 0.f}, ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();
	ImGui::PopStyleVar(3);
}

void Editor::DrawMenuBar() {
	if (ImGui::BeginMainMenuBar()) {
		auto& window = MM::Get<Window>();

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save Project", "Ctrl + S")) {
				window.SimulateKeyEvent(EventKey{.key = GLFW_KEY_S, .action = GLFW_PRESS, .mods = GLFW_MOD_CONTROL});
			}
			if (ImGui::MenuItem("Open Project", "Ctrl + O")) {
				window.SimulateKeyEvent(EventKey{.key = GLFW_KEY_O, .action = GLFW_PRESS, .mods = GLFW_MOD_CONTROL});
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo", "Ctrl + Z")) {
				window.SimulateKeyEvent(EventKey{.key = GLFW_KEY_Z, .action = GLFW_PRESS, .mods = GLFW_MOD_CONTROL});
			}
			if (ImGui::MenuItem("Redo", "Ctrl + Y")) {
				window.SimulateKeyEvent(EventKey{.key = GLFW_KEY_Y, .action = GLFW_PRESS, .mods = GLFW_MOD_CONTROL});
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Delete Entity", "Del")) {
				window.SimulateKeyEvent(EventKey{.key = GLFW_KEY_DELETE, .action = GLFW_PRESS});
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window")) {
			if (ImGui::MenuItem("Inspector", "F1")) {
				window.SimulateKeyEvent(EventKey{.key = GLFW_KEY_F1, .action = GLFW_PRESS});
			}
			if (ImGui::MenuItem("Hierarchy", "F2")) {
				window.SimulateKeyEvent(EventKey{.key = GLFW_KEY_F2, .action = GLFW_PRESS});
			}
			if (ImGui::MenuItem("Content Browser", "F3")) {
				window.SimulateKeyEvent(EventKey{.key = GLFW_KEY_F3, .action = GLFW_PRESS});
			}
			if (ImGui::MenuItem("History", "F4")) {
				window.SimulateKeyEvent(EventKey{.key = GLFW_KEY_F4, .action = GLFW_PRESS});
			}
			if (ImGui::MenuItem("History", "F5")) {
				window.SimulateKeyEvent(EventKey{.key = GLFW_KEY_F5, .action = GLFW_PRESS});
			}
			if (ImGui::MenuItem("Fullscreen", "F11")) {
				window.SimulateKeyEvent(EventKey{.key = GLFW_KEY_F11, .action = GLFW_PRESS});
			}
			ImGui::EndMenu();
		}
	}
	ImGui::EndMainMenuBar();
}

void Editor::HandleKeyPress(const EventKeyPress& e) {
	switch (e.key) {
	case GLFW_KEY_F1:
		mInspector.ToggleVisibility();
		break;
	case GLFW_KEY_F2:
		mHierarchy.ToggleVisibility();
		break;
	case GLFW_KEY_F3:
		mContentBrowser.ToggleVisibility();
		break;
	case GLFW_KEY_F4:
		mHistory.ToggleVisibility();
		break;
	case GLFW_KEY_F5:
		mViewport.ToggleVisibility();
		break;
	case GLFW_KEY_F11:
		mShowEditor = !mShowEditor;
		break;
	}
}

}  // namespace JR