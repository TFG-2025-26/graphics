Entities = {

  -- Primera entidad: la c mara
  {
    Id = 1,
    Name = "CameraEntity",
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
  
  {
    Id = 2,
    Name = "Menu",
    Components = {
      {
        Name = "MAIN_MENU",
        Arguments = {
        }
      }
    }
  },

  {
    Id = 99,
    Name = "UI_Example",
    Components = {
      {
        Name = "UI",
        Arguments = {
          Position = "100|200|0",
          Size = "0.2|0.1",
          Color = "1|0|0",
          Name = "TestOverlay",
          Material = "BaseWhite",
          Font = "OpenSans.ttf",
          Text = "Pulsa Enter para continuar",
          CharHeight = "40.0"
        }
      }
    }
  }

}