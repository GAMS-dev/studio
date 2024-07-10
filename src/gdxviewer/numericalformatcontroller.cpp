/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "numericalformatcontroller.h"
#include <numerics/doubleformatter.h>
#include <settings.h>

namespace gams {
namespace studio {
namespace gdxviewer {

const QString NumericalFormatController::svFull = "Full";

NumericalFormatController::NumericalFormatController()
{

}

void NumericalFormatController::initFormatComboBox(QComboBox *cb)
{
    cb->clear();
    cb->addItem("g-format", numerics::DoubleFormatter::g);
    cb->addItem("f-format", numerics::DoubleFormatter::f);
    cb->addItem("e-format", numerics::DoubleFormatter::e);
    cb->setCurrentIndex(Settings::settings()->toInt(SettingsKey::skGdxDefaultFormat));
    cb->setToolTip("<html><head/><body><p>Display format for numerical values:</p>"
                   "<p><span style=' font-weight:600;'>g-format:</span> The display format is chosen automatically:  <span style=' font-style:italic;'>f-format</span> for numbers closer to one and  <span style=' font-style:italic;'>e-format</span> otherwise. The value in the <span style=' font-style:italic;'>Precision</span> spin box specifies the number of significant digits. When precision is set to  <span style=' font-style:italic;'>Full</span>, the number of digits used is the least possible such that the displayed value would convert back to the value stored in GDX. Trailing zeros do not exist when <span style=' font-style:italic;'>precision=Full</span>.</p>"
                   "<p><span style=' font-weight:600;'>f-format:</span> Values are displayed in fixed format as long as appropriate. Large numbers are still displayed in scientific format. The value in the <span style=' font-style:italic;'>Precision</span> spin box specifies the number of decimals.</p>"
                   "<p><span style=' font-weight:600;'>e-format:</span> Values are displayed in scientific format. The value in the <span style=' font-style:italic;'>Precision</span> spin box specifies the number of significant digits. When precision is set to  <span style=' font-style:italic;'>Full</span>, the number of digits used is the least possible such that the displayed value would convert back to the value stored in GDX. Trailing zeros do not exist when <span style=' font-style:italic;'>precision=Full</span>.</p></body></html>");
}

void NumericalFormatController::initPrecisionSpinBox(QSpinBox *sb)
{
    sb->setSpecialValueText(svFull);
    sb->setValue(Settings::settings()->toInt(SettingsKey::skGdxDefaultPrecision));
    sb->setWrapping(true);
    sb->setToolTip("<html><head/><body><p>Specifies the number of decimals or the number of significant digits depending on the chosen format:</p><p><span style=' font-weight:600;'>"
                   "g-format:</span> Significant digits [1..17, Full]</p><p><span style=' font-weight:600;'>"
                   "f-format:</span> Decimals [0..14]</p><p><span style=' font-weight:600;'>"
                   "e-format:</span> Significat digits [1..17, Full]</p></body></html>");
}

bool NumericalFormatController::update(QComboBox *cbFormat, QSpinBox *sbPrecision, QCheckBox *cbSqZeroes, bool restoreSqZeroes)
{
    bool retRestoreSqZeroes = restoreSqZeroes;

    numerics::DoubleFormatter::Format format = static_cast<numerics::DoubleFormatter::Format>(cbFormat->currentData().toInt());
    if (format == numerics::DoubleFormatter::g || format == numerics::DoubleFormatter::e) {
        sbPrecision->setRange(numerics::DoubleFormatter::gFormatFull, 17);
        sbPrecision->setSpecialValueText(svFull);
    }
    else if (format == numerics::DoubleFormatter::f) {
        sbPrecision->setRange(0, 14);
        sbPrecision->setSpecialValueText("");
    }
    if (sbPrecision->text() == svFull && cbSqZeroes->isEnabled()) {
        if (!cbSqZeroes->isChecked())
            retRestoreSqZeroes = true;
        cbSqZeroes->setChecked(true);
        cbSqZeroes->setEnabled(false);
    }
    else if (sbPrecision->text() != svFull && !cbSqZeroes->isEnabled()) {
        cbSqZeroes->setEnabled(true);
        if (retRestoreSqZeroes) {
            cbSqZeroes->setChecked(false);
            retRestoreSqZeroes = false;
        }
    }
    return retRestoreSqZeroes;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
