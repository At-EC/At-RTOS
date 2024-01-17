#ifndef ATOS_VERSION_H_
#define ATOS_VERSION_H_

#define ATOS_BUILD_TIME                             "2024-26-17,20:26"

#define ATOS_VERSION_STRING                         "0.0.32.0"
#define ATOS_VERSION_PRODUCTION_RELEASE_NUMBER      (0u)
#define ATOS_VERSION_OFFICIAL_RELEASE_NUMBER        (0u)
#define ATOS_VERSION_CHANGES_NUMBER                 (32u)
#define ATOS_VERSION_CATEGORIES_NUMBER              (0u)

#define ATOS_VERSION_CATEGORIES_MASK                (0x0Fu)
#define ATOS_VERSION_CATEGORIES_POS                 (0u)

#define ATOS_VERSION_CHANGES_MASK                   (0x3FFu)
#define ATOS_VERSION_CHANGES_POS                    (4u)

#define ATOS_VERSION_OFFICIAL_RELEASE_MASK          (0x3FFu)
#define ATOS_VERSION_OFFICIAL_RELEASE_POS           (14u)

#define ATOS_VERSION_PRODUCTION_RELEASE_MASK        (0xFFu)
#define ATOS_VERSION_PRODUCTION_RELEASE_POS         (24u)

#endif
