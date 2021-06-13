#pragma once

class nao_presenter;

class nao_model {
    nao_presenter& _presenter;

    public:
    explicit nao_model(nao_presenter& presenter);
};
