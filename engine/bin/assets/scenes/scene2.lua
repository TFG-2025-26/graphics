Entities = {
  -- camara --
  {
    Id = 1,
    Name = "CameraEntity2",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "0|30|10",
          Rotation = "-0.7|0|0|1",
          Scale    = "1|1|1"
        }
      },
      {
        Name = "CAMERA",
        Arguments = {
          NearClip = "1",
          FarClip  = "10000"
        }
      },
      {
          Name = "TEST_COMPONENT",
          Arguments = {

          }
      }
    }
  },

  -- luz --
  {
    Id = 2,
    Name = "LightEntity",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "0|300|40",
          Rotation = "-0.316|0|0|0.949",
          Scale    = "1|1|1"
        }
      },
      {
        Name = "LIGHT",
        Arguments = {
          Color = "1|1|1",
          LightType = "DIRECTIONAL"
        }
      }
    }
  },

  -- Jugador --
  {
    Id = 3,
    Name = "SinbadEntity",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "0|2|6",
          Rotation = "0|0|0|1",
          Scale    = "0.5|0.5|0.5"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "Sinbad.mesh"
        }
      },
      {
        Name = "ANIMATOR",
        Arguments = {
          Animations = "Dance|RunBase|RunTop",
          Enabled = "true|false|false",
          Loop = "true|true|true"
        }
      },
      {
        Name = "COLLIDER",
        Arguments = {
        }
      },
      {
        Name = "RIGIDBODY",
        Arguments = {
          Mass    = "100",
          ShapeType = "BOX",           -- shapeType
          Length = "1|2|1",   -- length/size
          Radius = "1",
          Offset = "0|0|0",
          RBType= "DYNAMIC"
        }
      },
      {
        Name = "PLAYER_CONTROLLER",
        Arguments = {
          Speed = "5"
        }
      }
    }
  },

  -- Suelo --
  {
    Id = 4,
    Name = "Suelo",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "0|-2|0",
          Rotation = "0|0|0|1",
          Scale    = "0.25|0.01|0.25"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "cube.mesh",
          Material = "suelo"
        }
      },
      {
        Name = "COLLIDER",
        Arguments = {  
        }
      },
      {
        Name = "RIGIDBODY",
        Arguments = {
          Mass    = "100000000",
          ShapeType   = "BOX",           -- shapeType
          Length = "12|2|12",   -- length/size
          Offset = "0|0|0",
          RBType= "STATIC"
        }
      },
      {
        Name = "FLOOR",
        Arguments = {
        }
      }
    }
  },

  --Cocina--
  {
    Id = 5,
    Name = "KichenEntity",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "4|-1|6",
          Rotation = "0|0|0|1",
          Scale    = "2|2|2"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "Kichen.mesh",
          Material = "marmol"
        }
      }
    }
  },

  --Barrera--
  -- {
  --   Id = 6,
  --   Name = "BarreraEntity",
  --   Components = {
  --     {
  --       Name = "TRANSFORM",
  --       Arguments = {
  --         Position = "0|0|-1",
  --         Rotation = "0|0|0|1",
  --         Scale    = "0.2|0.02|0.01"
  --       }
  --     },
  --     {
  --       Name = "MESH",
  --       Arguments = {
  --         Mesh = "cube.mesh",
  --         Material = "marmol"
  --       }
  --     },
  --     {
  --       Name = "COLLIDER",
  --       Arguments = {
  --       } 
  --     },
  --     {
  --       Name = "RIGIDBODY",
  --       Arguments = {
  --         Mass    = "100000000",
  --         ShapeType   = "BOX",           -- shapeType
  --         Length = "10|1|0.5",   -- length/size
  --         Offset = "0|0|0",
  --         RBType= "STATIC"
  --       }
  --     }
  --   }
  -- },

  --Audio--
  {
    Id = 7,
    Name = "AudioEntity",
    Components = {
      {
        Name = "AUDIO_SOURCE",
        Arguments = {
          Sound = "SpiderMan2Pizza.mp3",
          Name = "Pizza",
          Volume = "0.05",
          Loop = "true",
          PlayOnStart = "true",
          Music = "true"
        }
      }
    }
  },

  -- Ingrediente: Tomate --
  {
    Id = 8,
    Name = "TomatoEntity",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "5|2|10",
          Rotation = "0|0|0|1",
          Scale    = "0.5|0.5|0.5"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "Tomate.mesh",
          Material = "Tomate"
        }
      },
      {
        Name = "COLLIDER",
        Arguments = {  
        }
      },
      {
        Name = "RIGIDBODY",
        Arguments = {
          Mass    = "10",
          ShapeType   = "BOX",           -- shapeType
          Length = "1|1|1",   -- length/size
          Offset = "0|0|0",
          RBType= "KINEMATIC"
        }
      },
      {
        Name = "INGREDIENT",
        Arguments = {
          Type = "tomato"
        }
      }
    }
  },

  -- Ingrediente: Queso --
  {
    Id = 9,
    Name = "CheeseEntity",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "-3|4|10",
          Rotation = "0|0|0|1",
          Scale    = "0.5|0.5|0.5"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "Queso.mesh",
          Material = "Queso"
        }
      },
      {
        Name = "COLLIDER",
        Arguments = {  
        }
      },
      {
        Name = "RIGIDBODY",
        Arguments = {
          Mass    = "10",
          ShapeType   = "BOX",           -- shapeType
          Length = "1|2|1",   -- length/size
          Offset = "0|0|0",
          RBType= "KINEMATIC"
        }
      },
      {
        Name = "INGREDIENT",
        Arguments = {
          Type = "cheese"
        }
      }
    }
  },

  -- Ingrediente: Peperoni --
  {
    Id = 10,
    Name = "PeperoniEntity",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "0|2|10",
          Rotation = "0|0|1|1",
          Scale    = "0.5|0.5|0.5"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "Peperoni.mesh",
          Material = "Peperoni"
        }
      },
      {
        Name = "COLLIDER",
        Arguments = {  
        }
      },
      {
        Name = "RIGIDBODY",
        Arguments = {
          Mass    = "10",
          ShapeType   = "BOX",           -- shapeType
          Length = "1|1|1",   -- length/size
          Offset = "0|0|0",
          RBType= "KINEMATIC"
        }
      },
      {
        Name = "INGREDIENT",
        Arguments = {
          Type = "peperoni"
        }
      }
    }
  },

  -- Masa --
  {
    Id = 11,
    Name = "MasaEntity",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "4.5|2|3.5",
          Rotation = "0|0|0|1",
          Scale    = "0.7|0.7|0.7"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "Masa.mesh",
          Material = "Masa"
        }
      },
      {
        Name = "COLLIDER",
        Arguments = {  
        }
      },
      {
        Name = "RIGIDBODY",
        Arguments = {
          Mass    = "10",
          ShapeType   = "BOX",           -- shapeType
          Length = "1|1|1",   -- length/size
          Offset = "0|0|0",
          RBType= "KINEMATIC"
        }
      },
      {
        Name = "CREATE_PIZZA",
        Arguments = {  
        }
      }
    }
  },

  -- Papelera --
  {
    Id = 12,
    Name = "PapeleraEntity",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "-8|2|10",
          Rotation = "0|1|0|1",
          Scale    = "1|1|1"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "Papelera.mesh",
          Material = "Mesa"
        }
      },
      {
        Name = "COLLIDER",
        Arguments = {  
        }
      },
      {
        Name = "RIGIDBODY",
        Arguments = {
          Mass    = "10",
          ShapeType   = "BOX",           -- shapeType
          Length = "1|1|2.3",   -- length/size
          Offset = "0|0|0",
          RBType= "KINEMATIC"
        }
      },
      {
        Name = "DELETE_PIZZA",
        Arguments = {  
        }
      }
    }
  },
  -- Horno --
  {
    Id = 13,
    Name = "HornoEntity",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "-8|2|3",
          Rotation = "0|0|0|1",
          Scale    = "1.3|1.3|1.3"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "Horno.mesh",
          Material = "Mesa"
        }
      },
      {
        Name = "COLLIDER",
        Arguments = {  
        }
      },
      {
        Name = "RIGIDBODY",
        Arguments = {
          Mass    = "10",
          ShapeType   = "BOX",           -- shapeType
          Length = "1.3|1.3|1.3",   -- length/size
          Offset = "0|0|0",
          RBType= "KINEMATIC"
        }
      }, 
      {
        Name = "AUDIO_SOURCE",
        Arguments = {
          Sound = "ovenRing.mp3",
          Name = "oven",
          Volume = "0",
          Loop = "false",
          PlayOnStart = "false",
          Music = "false"
        }
      },
      {
        Name = "UI",
        Arguments = {
          Position = "0|300|0",
          Size = "0.2|0.1",
          Color = "1|0|0",
          Name = "HornoOverlay",
          Material = "BaseWhite",
          Font = "OpenSans.ttf",
          Text = "Tiempo: 0",
          CharHeight = "20.0"
        }
      },
      {
        Name = "OVEN",
        Arguments = {  
        }
      }
    }
  },

  -- Mesa --
  {
    Id = 14,
    Name = "MesaEntity",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "-8|2|-8",
          Rotation = "0|0|0|1",
          Scale    = "0.4|0.4|0.4"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "Mesa.mesh",
          Material = "Mesa"
        }
      }
    }
  },

  -- Mesa1 --
  {
    Id = 15,
    Name = "MesaEntity1",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "0|2|-8",
          Rotation = "0|0|0|1",
          Scale    = "0.4|0.4|0.4"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "Mesa.mesh",
          Material = "Mesa"
        }
      }
    }
  },

  -- Mesa2 --
  {
    Id = 16,
    Name = "MesaEntity2",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "8|2|-8",
          Rotation = "0|0|0|1",
          Scale    = "0.4|0.4|0.4"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "Mesa.mesh",
          Material = "Mesa"
        }
      }
    }
  },
    -- Mesa3 --
    {
      Id = 17,
      Name = "MesaEntity3",
      Components = {
        {
          Name = "TRANSFORM",
          Arguments = {
            Position = "-8|2|-2",
            Rotation = "0|0|0|1",
            Scale    = "0.4|0.4|0.4"
          }
        },
        {
          Name = "MESH",
          Arguments = {
            Mesh = "Mesa.mesh",
            Material = "Mesa"
          }
        }
      }
    },
    -- Mesa4 --
  {
    Id = 15,
    Name = "MesaEntity4",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "0|2|-2",
          Rotation = "0|0|0|1",
          Scale    = "0.4|0.4|0.4"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "Mesa.mesh",
          Material = "Mesa"
        }
      }
    }
  },

      -- Mesa5 --
  {
    Id = 19,
    Name = "MesaEntity5",
    Components = {
      {
        Name = "TRANSFORM",
        Arguments = {
          Position = "8|2|-2",
          Rotation = "0|0|0|1",
          Scale    = "0.4|0.4|0.4"
        }
      },
      {
        Name = "MESH",
        Arguments = {
          Mesh = "Mesa.mesh",
          Material = "Mesa"
        }
      }
    }
  },

  {
    Id = 20,
    Name = "UI_Reputacion",
    Components = {
      {
        Name = "UI",
        Arguments = {
          Position = "0|150|0",
          Size = "0.2|0.1",
          Color = "1|0|0",
          Name = "ReputacionOverlay",
          Material = "BaseWhite",
          Font = "OpenSans.ttf",
          Text = "Reputacion: 0",
          CharHeight = "20.0"
        }
      }
    }
  },
  {
    Id = 21,
    Name = "UI_Dinero",
    Components = {
      {
        Name = "UI",
        Arguments = {
          Position = "0|170|0",
          Size = "0.2|0.1",
          Color = "1|0|0",
          Name = "DineroOverlay",
          Material = "BaseWhite",
          Font = "OpenSans.ttf",
          Text = "Dinero: 0",
          CharHeight = "20.0"
        }
      }
    }
  },
  {
    Id = 22,
    Name = "UI_Tiempo",
    Components = {
      {
        Name = "UI",
        Arguments = {
          Position = "0|190|0",
          Size = "0.2|0.1",
          Color = "1|0|0",
          Name = "TiempoOverlay",
          Material = "BaseWhite",
          Font = "OpenSans.ttf",
          Text = "Tiempo: 0",
          CharHeight = "20.0"
        }
      }
    }
  },

  -- GameManager --
  {
    Id = 23,
    Name = "GameManager",
    Components = {
      {
        Name = "GAME_MANAGER",
        Arguments = {
          Mesas ="MesaEntity|MesaEntity1|MesaEntity2|MesaEntity3|MesaEntity4|MesaEntity5",
          Ingredientes = "tomato|cheese|peperoni",
          UI_Rep = "UI_Reputacion",
          UI_Mon = "UI_Dinero",
          UI_Ti = "UI_Tiempo"
        }
      }
    }
  }
}

-- Aqu podr as a adir m s cosas, como:
--     animatorSinbad->setAnimationEnabled("Dance", true);
--     animatorSinbad->setAnimationLoop("Dance", true);
--  Se podr a reflejar en la tabla "Animator" o en un script aparte:
--  Por ejemplo:
-- Animations = {
--   { EntityId = 3, Name="Dance", Enabled=true, Loop=true }
-- }
