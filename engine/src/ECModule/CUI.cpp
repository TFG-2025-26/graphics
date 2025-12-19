#include "CUI.h"

#include "ComponentArguments.h"

#include "Vector2.h"
#include "Vector3.h"

#include "UIManager.h"
#include "RenderManager.h"
#include "Entity.h"
#include "FluxError.h"

flux_ec::CUI::~CUI()
{
	delete _pos;
	delete _size;
	delete _color;
}

bool flux_ec::CUI::init(flux_script::ComponentArguments* args)
{
	_pos = new flux_utils::Vector3();
	_size = new flux_utils::Vector2();
	_color = new flux_utils::Vector3();

	*_pos = args->getValueToVector3("Position");
	*_size = args->getValueToVector2("Size");
	*_color = args->getValueToVector3("Color");

	_overlayName = args->getValueToString("Name");
	_material = args->getValueToString("Material");
	_font = args->getValueToString("Font");
	_text = args->getValueToString("Text");
	_charHeight = args->getValueToFloat("CharHeight");


	flux_render::UIManager* ui = flux_render::RenderManager::instance()->getUIManager();
	if (ui == nullptr) {
		throwFluxError(false, "No se ha podido encontrar el UI Manager");
	}

	if (!ui->registerComponent(this)) {
		throwFluxError(false, "Error en el registro de una UI en el UI Manager");
	}

	return true;
}

void flux_ec::CUI::update(float dt)
{
	// ...
}

flux_utils::Vector3 flux_ec::CUI::getPosition() const
{
	return *_pos;
}

flux_utils::Vector2 flux_ec::CUI::getSize() const
{
	return *_size;
}

flux_utils::Vector3 flux_ec::CUI::getColor() const
{
	return *_color;
}

std::string flux_ec::CUI::getMaterial() const
{
	return _material;
}

std::string flux_ec::CUI::getOverlayName() const
{
	return _overlayName;
}

std::string flux_ec::CUI::getFont() const
{
	return _font;
}

std::string flux_ec::CUI::getText() const
{
	return _text;
}

float flux_ec::CUI::getCharHeight() const
{
	return _charHeight;
}

std::string flux_ec::CUI::getSceneID() const
{
	return getOwner()->getSceneID();
}

void flux_ec::CUI::setText(std::string nt) {
	_text = nt;
	flux_render::RenderManager::instance()->getUIManager()->updateText(getOwner()->getSceneID(),this);
}
