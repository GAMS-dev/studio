/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
 */
#ifndef GAMS_STUDIO_GAMSINSTPROCESS_H
#define GAMS_STUDIO_GAMSINSTPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {
namespace process {

class GamsInstProcess : public AbstractGamsProcess
{
    Q_OBJECT

public:
    GamsInstProcess(QObject *parent = nullptr);
    void execute() override;

    QStringList configPaths() const {
        return mConfig;
    }

    QStringList dataPaths() const {
        return mData;
    }

    QStringList defaultParameters() const override {
        return {"-listdirs"};
    }

private slots:
    void newData(const QByteArray &data);

private:
    bool isData = false;
    QStringList mData;
    QStringList mConfig;
};

} // namespace process
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GAMSINSTPROCESS_H
